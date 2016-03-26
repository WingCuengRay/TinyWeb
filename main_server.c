#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include "webServer.h"
#include "wrap.h"

void doit(int fd);

int main(int argc, char **argv)
{
	int listenfd, connfd, port, clientlen;
	struct sockaddr_in clientaddr;

	if(argc !=2)
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(-1);
	}
	port = atoi(argv[1]);

	listenfd = open_listenfd(port);
	while(1)
	{
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t*)&clientlen);
		doit(connfd);
		Close(connfd);
	}	
	
	return 0;
}


void doit(int fd)
{
	rio_t rio;
	char buf[MAXLINE];
	char method[MAXLINE], uri[MAXLINE], version[MAXLINE];

	rio_readinitb(&rio, fd);
	rio_readlineb(&rio, buf, sizeof(buf));
	sscanf(buf, "%s %s %s", method, uri, version);
	if(strcasecmp(method, "GET"))		//若不相等（返回０），则未实现除了GET之外的命令
	{
		clienterror(fd, method, "501", "Not Implemented",
				"Tiny does not implement this method");
		return;
		
	}
	read_request_hdrs(&rio);

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
