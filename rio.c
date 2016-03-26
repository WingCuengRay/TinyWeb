#include "rio.h"
#include <unistd.h>
#include <errno.h>
#include <memory.h>


static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
/**
 * @brief rio_read  RIO--Robust I/O包 底层读取函数。当缓冲区充足时，此函数直接返回缓
 *					冲区的数据给上层读取函数；当缓冲区不足时，该函数自动通过系统调用
 *					从文件中读取最大数量的字节到缓冲区，再返回缓冲区数据给上层函数
 *
 * @param rp		rio_t，里面包含了文件描述符和其对应的缓冲区数据
 * @param usrbuf	读取的目的地址
 * @param n			读取的字节数量
 *
 * @returns			返回真正读取到的字节数（<=n）
 */
{
	int cnt;

	while(rp->rio_cnt <= 0)		//为什么是while而不是if
	{
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if(rp->rio_cnt < 0)
		{
			if(errno != EINTR)	//如果不是因为中断的原因导致read错误的话，就返回负值表示读取失败
				return -1;
		}
		else if(rp->rio_cnt == 0)	//读取到了EOF
			return 0;
		else
			rp->rio_bufptr = rp->rio_buf;		//重置bufptr指针，令其指向第一个未读取字节
	}

	cnt = n;
	if((size_t)rp->rio_cnt < n)		
		cnt = rp->rio_cnt;
	memcpy(usrbuf, rp->rio_bufptr, n);
	rp->rio_bufptr += cnt;		//读取后需要更新指针
	rp->rio_cnt -= cnt;			//未读取字节也会减少

	return cnt;
}


ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
/**
 * @brief rio_readnb	供用户使用的读取函数。从缓冲区中读取最大maxlen字节数据
 *
 * @param rp			rio_t，文件描述符与其对应的缓冲区
 * @param usrbuf		void *, 目的地址
 * @param n				size_t, 用户想要读取的字节数量
 *
 * @returns				真正读取到的字节数。读到EOF返回0,读取失败返回-1。
 */
{
	size_t leftcnt = n;
	ssize_t nread;
	char *buf = (char *)usrbuf;

	while(leftcnt > 0)
	{
		if((nread = rio_read(rp, buf, n)) < 0)
		{
			if(errno == EINTR)		//如果遇到中断，则代表没读到数据，所以将nread清零
				nread = 0;
			else 
				return -1;
		}
		leftcnt -= nread;
		buf += nread;
	}

	return n-leftcnt;
}


ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
/**
 * @brief rio_readlineb	读取一行的数据，遇到'\n'结尾代表一行
 *
 * @param rp			rio_t包
 * @param usrbuf		用户地址，即目的地址
 * @param maxlen		size_t, 一行最大的长度。若一行数据超过最大长度，则以'\0'截断
 *
 * @returns				真正读取到的字符数量
 */
{
	size_t n;
	int rd;
	char c, *bufp = (char *)usrbuf;

	for(n=1; n<maxlen; n++)		//n代表已接收字符的数量
	{
		if((rd=rio_read(rp, &c, 1)) == 1)
		{
			*bufp++ = c;
			if(c == '\n')
				break;
	//		if(errno == EINTR)
	//			continue;
	//		else 
	//			return -1;
		}
		else if(rd == 0)		//没有接收到数据
		{
			if(n == 1)			//如果第一次循环就没接收到数据，则代表无数据可接收
				return 0;
			else
				break;
		}
		else					//为什么这里不要判断中断
			return -1;
	}
	*bufp = 0;

	return n;
}


ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
	size_t nleft = n;
	ssize_t nwritten;
	char *bufp = (char *)usrbuf;

	while(nleft > 0)
	{
		if((nwritten = write(fd, bufp, nleft)) <= 0)
		{
			if(errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		bufp += nwritten;
		nleft -= nwritten;
	}

	return n;
}




void rio_readinitb(rio_t *rp, int fd)
/**
 * @brief rio_readinitb		rio_t 结构体初始化,并绑定文件描述符与缓冲区
 *
 * @param rp				rio_t结构体
 * @param fd				文件描述符
 */
{
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;

	return;
}
