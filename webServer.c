#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "wrap.h"
#include "rio.h"
#include "webServer.h" 


static char *Index(char *uri, char ch);

void clienterror(int fd, char *cause, char *errnum,
				char *shortmsg, char *longmsg)
/**
 * @brief clienterror	发送HTTP访问错误的报文
 *
 * @param fd
 * @param cause			char *, 原因说明
 * @param errnum		char *, 错误号
 * @param shortmsg		char *, 错误简短说明
 * @param longmsg		cahr *, 错误详细说明
 **/
{
	char head[MAXLINE], body[MAXBUF];
	
	//HTTP相应报文的内容
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);


	//HTTP相应报文头部
	sprintf(head, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	rio_writen(fd, head, strlen(head));
	sprintf(head, "Content-type: text/html\r\n");
	rio_writen(fd, head, strlen(head));
	sprintf(head, "Conetent-length: %d\r\n\r\n", (int)strlen(body));
	rio_writen(fd, head, strlen(head));

	rio_writen(fd, body, strlen(body));
}


void read_request_hdrs(rio_t *rp)
/**
 * @brief read_request_hdrs 读取HTTP请求报头（不包括请求行）并舍弃
 *
 * @param rp				rio_t *, 文件描述符与其对应的缓冲区
 **/
{
	char buf[MAXBUF] = {0};

	rio_readlineb(rp, buf, sizeof(buf));
	while(strcmp(buf, "\r\n"))
	{
		rio_readlineb(rp, buf, sizeof(buf));
#ifdef __DEBUG
		printf("%s", buf);			//输出请求报文头，易于调试
#endif
	}
#ifdef __DEBUG
	printf("read_request_hdrs() return!\n");
#endif
}



int parse_uri(char *uri, char *filename, char *args)
/**
 * @brief parse_uri		分析HTTP请求行，将uri分解为文件名和可选参数
 *
 * @param uri			char *, 要分析的 uri
 * @param filename		char *, HTTP请求报文请求的文件名
 * @param args			char *, 可执行文件的参数（若为服务动态内容）
 *
 * @returns				返回文件的类型，1代表静态内容，0代表动态内容(cgi-bin目录下的文件皆为动态)
 **/
{
	int ftype;
	char *p;

	//根据uri中是否有 cgi-bin 目录判断是否为静态/动态类型
	if(!strstr(uri, "cgi-bin"))
		ftype = 1;
	else
		ftype = 0;

	if(ftype)		//静态内容
	{
		strcpy(args, "");
		strcpy(filename, ".");		//默认在当前目录下
		strcat(filename, uri);		
		if(uri[strlen(uri)-1] == '/')
			strcat(filename, "index.html");
#ifdef __DEBUG
		printf("parse_uri() filename:%s\n", filename);
#endif
	}
	else			//动态内容
	{
		p = Index(uri, '?');		//找到文件名与参数的分隔符 '?'
		if(p != NULL)
		{
			strcpy(args, p+1);	
			*p = 0;					//截断文件名与参数
		}
		else
			strcpy(args, "");
		strcpy(filename, ".");
		strcat(filename, uri);
	}

	return ftype;
}



void server_static(int sockfd, char *filename, size_t fsize)
/**
 * @brief server_static		提供静态服务——通过socket将文件内容发送给客户端
 *
 * @param sockfd				int, socket 文件描述符
 * @param filename			char *, 要发送的文件名
 * @param fsize				int, 文件的大小
 * @pre_condition			文件filename非可执行文件且存在于当前目录下
 **/
{
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXBUF];
	
	//发送HTTP响应报文头部
	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sContent-length: %ld", buf, fsize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	rio_writen(sockfd, buf, strlen(buf));

	//使用包裹函数，打开文件失败便会结束进程，robust性不足
	srcfd = Open(filename, O_RDONLY, 0);				//若文件已存在，则第三个参数无意义
	//将文件映射到虚拟内存中（实际上是将文件的内容复制到了内存）
	srcp = Mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	Close(srcfd);
	Rio_writen(sockfd, srcp, fsize);
#ifdef __DEBUG
	printf("%s", srcp);
#endif
	Munmap(srcp, fsize);
	
	return;
}

extern char **environ;
void server_dynamic(int sockfd, char *filename, char *cgiargs)
/**
 * @brief server_dynamic	提供动态服务
 *
 * @param sockfd			socket文件描述符
 * @param filename			可执行文件名
 * @param args				参数
 **/
{
	char buf[MAXLINE];
	char *emptylist[] = { NULL };

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	rio_writen(sockfd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	rio_writen(sockfd, buf, strlen(buf));

	if(Fork() == 0)		//子进程才会执行
	{
		setenv("QUERY_STRING", cgiargs, 1);
		Dup2(sockfd, STDOUT_FILENO);				//将标准输出重定向到sockfd,这样新程序的输出便写入了sockfd中
		Execve(filename, emptylist, environ);		//执行新的程序
	}
	Wait(NULL);			//阻塞等待子进程结束，并回收资源
}


void get_filetype(char * const filename, char *filetype)
/**
 * @brief get_filetype	根据文件的扩展名获得其类型
 *
 * @param filename		文件名
 * @param filetype		存储类型的地方
 **/
{
	if(strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if(strstr(filetype, ".gif"))
		strcpy(filetype, "image/gif");
	else if(strstr(filetype, ".jpeg"))
		strcpy(filetype, "imagge/jpeg");
	else 
		strcpy(filetype, "text/plain");
}


static char *Index(char *uri, char ch)
{
	char *p;
	for(p=uri; *p!='\0'; p++)
	{
		if(*p == ch)
			return p;
	}

	return NULL;
}





