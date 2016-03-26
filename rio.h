#ifndef __RIO_H
#define __RIO_H

#include <unistd.h>

#define RIO_BUFSIZE		8092
typedef struct
{
	int rio_fd;		//与缓冲区绑定的文件描述符的编号
	int rio_cnt;	//缓冲区中还未读取的字节数
	char *rio_bufptr;	//当前下一个未读取字符的地址
	char rio_buf[RIO_BUFSIZE];
}rio_t;

void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

#endif
