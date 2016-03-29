#include "wrap.h"
#include "pool.h"


void init_pool(int listenfd, Pool *p)
/**
 * @brief init_pool		根据监听套接字初始化pool池
 *
 * @param listenfd
 * @param p
 **/
{
	int i;

	p->maxi = -1;
	for(i=0; i<FD_SETSIZE; i++)
		p->clientfd[i] = -1;

	//将服务器的监听套接字加入监听集合中
	p->maxfd = listenfd;
	FD_ZERO(&p->read_set);
	FD_SET(listenfd, &p->read_set);

	return;
}


void add_client(int connfd, Pool *p)
/**
 * @brief add_client	将一个客户端发起的连接套接字加入套接字池
 *
 * @param connfd		连接套接字的文件描述符
 * @param p
 **/
{
	int i;

	p->nready--;		//已处理监听套接字，所以“可读”数量减小1	
	for(i=0; i<FD_SETSIZE; i++)
	{
		if(p->clientfd[i] < 0)	//未被使用
		{
			p->clientfd[i] = connfd;
			rio_readinitb(&p->clientrio[i], connfd);

			FD_SET(connfd, &p->read_set);
			//更新两个表示“最大值”的变量
			if(connfd > p->maxfd)
				p->maxfd = connfd;
			if(i > p->maxi)
				p->maxi = i;
			break;
		}
	}
	if(i == FD_SETSIZE)
		app_error("add_client error: Too many clients.");
	return;
}


