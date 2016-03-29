#ifndef __POOL_H
#define __POOL_H

#include <sys/select.h>
#include "rio.h"

typedef struct
{
	int maxfd;			//监听读集合中最大的文件描述符
	fd_set read_set;	//所有要监听的描述符集合
	fd_set ready_set;	//已准备好，可读的描述符集合
	int nready;		//可读文件描述符的数量
	int maxi;		//客户端已连接套接字的最大描述符值
	int clientfd[FD_SETSIZE];
	rio_t clientrio[FD_SETSIZE];
}Pool;


void init_pool(int listenfd, Pool *p);
void add_client(int connfd, Pool *p);

#endif
