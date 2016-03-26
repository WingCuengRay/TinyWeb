#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include "wrap.h"
#include "rio.h"




ssize_t Open(char *filename, int flags, mode_t mode)
{
	int fd;
	char buf[50];
	fd = open(filename, flags, mode);

	if(fd < 0)
	{
		sprintf(buf, "Open %s error\n", filename);
		unix_error(buf);
	}
	return fd;
}


int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int fd = accept(sockfd, addr, addrlen);

	if(fd < 0)
		unix_error("Accept error!\n");

	return fd;
}


int Connect(int sockfd, struct sockaddr *addr, socklen_t addrlen)
{
	int res = connect(sockfd, addr, addrlen);
	if(res != 0)
		unix_error("Connect error!");

	return res;
}


int Close(int sockfd)
{
	int res = close(sockfd);
	if(res < 0)
		unix_error("Close error!");

	return res;
}


void *Mmap(void *addr, size_t length, int prot, int flags,
			int fd, off_t offset)
{
	void *p = mmap(addr, length, prot, flags, fd, offset);
	
	if((int)p == -1)
		unix_error("Mmap error!\n");
	return p;
}


int Munmap(void *addr, size_t length)
{
	int res = munmap(addr, length);
	if(res == -1)
		unix_error("Munmap error!\n");
	
	return res;
}

int Fork()
{
	int res = fork();
	if(res == -1)
		unix_error("Fork error!\n");
	return res;
}

int Execve(char *filename, char *argv[], char *envp[])
{
	int res = execve(filename, argv, envp);

	if(res == -1)
		unix_error("Execve error!");
	return res;
}

int Wait(int *status)
{
	int res = wait(status);
	if(res == -1)
		unix_error("Wait Error!\n");

	return res;
}

int Dup2(int oldfd, int newfd)
{
	int res = dup2(oldfd, newfd);
	if(res == -1)
		unix_error("Dup2 error!\n");
	return res;
}

int open_clientfd(char *hostname, int port)
/**
 * @brief open_clientfd  建立一个客户端套接字，用于与服务器通信
 *
 * @param hostname		 目的服务器主机名
 * @param port			 目的服务器端口
 *
 * @returns				 成功返回文件描述符，失败返回-1
 **/
{
	int clientfd;
	struct hostent *hp;
	struct sockaddr_in clientaddr;

	if((clientfd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	hp = gethostbyname(hostname);
	if(hp == NULL)
		return -2;
	bzero(&clientaddr, sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	//hp->h_addr_list[]的每一个元素都是对应于hostname的一个网络序的ip地址，所以直接复制即可  
	bcopy((char *)hp->h_addr_list[0], &clientaddr.sin_addr, hp->h_length);
	clientaddr.sin_port = htons(port);

	if(connect(clientfd, (SA *)&clientaddr, sizeof(clientaddr)) < 0)
		return -1;
	return clientfd;
}


int open_listenfd(int port)
/**
 * @brief open_listenfd		建立一个端口号为port，接受任意ip的监听套接字
 *
 * @param port				端口号
 *
 * @returns					成功返回文件描述符，失败返回-1
 */
{
	int srvfd;
	struct sockaddr_in srvaddr;
	int optval = 1;


	if((srvfd=socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		return -1;

	//设置允许端口重用
	if(setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, 
				(const void *)&optval, sizeof(optval)))
		return -1;

	bzero(&srvaddr, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//接受任意的外来的ip地址
	srvaddr.sin_port = htons(port);

	if((bind(srvfd, (SA *)&srvaddr, sizeof(srvaddr))) < 0)
	{
		Close(srvfd);
		return -2;
	}
	
	//listen 函数可以将套接字转化成监听套接字
	if((listen(srvfd, LISTENQ)) < 0)
	{
		close(srvfd);
		return -2;
	}

	return srvfd;
}



int Rio_writen(int fd, void *usrbuf, size_t n)
{
	int res = rio_writen(fd, usrbuf, n);
	if(res == -1)
		unix_error("Rio_writen error!\n");
	return res;
}
