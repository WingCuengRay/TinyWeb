#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include "webServer.h"
#include "wrap.h"
#include "pool.h"

void doit(int fd, rio_t *rp);
void serve_clients(Pool *p);

int main(int argc, char **argv)
{
	int listenfd, connfd, port, clientlen;
	struct sockaddr_in clientaddr;
	static Pool pool;

	if(argc !=2)
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(-1);
	}
	port = atoi(argv[1]);
	clientlen = sizeof(clientaddr);

	listenfd = open_listenfd(port);
	init_pool(listenfd, &pool);		//将监听套接字与缓冲池绑定（初始化）
	while(1)
	{
		pool.ready_set = pool.read_set;		//每次select前恢复监听套接字集
		pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

		if(FD_ISSET(listenfd, &pool.ready_set))
		{
			connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
			add_client(connfd, &pool);
		}
		serve_clients(&pool);
	}	
	
	return 0;
}



void serve_clients(Pool *p)
/**
 * @brief serve_clients　	服务客户端的请求——连接套接字
 *
 * @param p					套接字池
 **/
{
	int connfd;

	for(int i=0;i<=p->maxi && p->nready>0; i++)
	{
		rio_t *rp = &p->clientrio[i];
		connfd = p->clientfd[i];
		
		if((connfd>0) && (FD_ISSET(connfd, &p->ready_set)))
		{
			p->nready--;
			doit(connfd, rp);
			Close(connfd);		//服务完毕，关闭套接字
			FD_CLR(connfd, &p->read_set);
			p->clientfd[i] = -1;
		}
	}
	
	return;
}


void doit(int fd, rio_t *rp)
{
	char buf[MAXLINE];
	char method[MAXLINE], uri[MAXLINE], version[MAXLINE];

	rio_readinitb(rp, fd);
	rio_readlineb(rp, buf, sizeof(buf));
	sscanf(buf, "%s %s %s", method, uri, version);
	if(strcasecmp(method, "GET"))		//若不相等（返回０），则未实现除了GET之外的命令
	{
		clienterror(fd, method, "501", "Not Implemented",
				"Tiny does not implement this method");
		return;
		
	}
	read_request_hdrs(rp);

	//从GET请求行中解析uri
	char filename[MAXLINE], cgiargs[MAXLINE];
	struct stat fstat;
	int is_static = parse_uri(uri, filename, cgiargs);

#ifdef __DEBUG
	printf("After parse_uri()!\n");
#endif
	if(stat(filename, &fstat) < 0)		//获取文件信息
	{
#ifdef __DEBUG
		printf("in if stat()\n");
#endif
		clienterror(fd, filename, "404", "Not found",
				"Tiny couldn't find the file");
		return;
	}
#ifdef __DEBUG
	printf("After stat(filename, &fstat)\n");
#endif

	if(is_static)		//静态服务
	{
		//判断是否为普通文件,且拥有者可以能够执行读文件
		if(!(S_ISREG(fstat.st_mode)) || !(S_IRUSR & fstat.st_mode))
		{
			clienterror(fd , filename, "403", "Forbidden", 
					"Tiny couldn't read the file");
			return;
		}
		server_static(fd, filename, fstat.st_size);
	}
	else				//动态服务
	{
		if(!(S_ISREG(fstat.st_mode)) || !(S_IXUSR & fstat.st_mode))
		{
			clienterror(fd, filename, "403", "Forbidden",
					"Tiny couldn't execute the file");
			return;
		}
		server_dynamic(fd, filename, cgiargs);
	}
#ifdef __DEBUG
	printf("Server static/dynamic finished!\n");
#endif
}
