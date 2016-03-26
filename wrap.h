#ifndef __WRAP_H
#define __WRAP_H

#include <netinet/in.h>
#include "err.h"

#define LISTENQ 1024
typedef struct sockaddr SA;

ssize_t Open(char *filename, int flags, mode_t mode);
int	Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int Close(int sockfd);
void *Mmap(void *addr, size_t length, int prot, int flags,
			int fd, off_t offset);
int Munmap(void *addr, size_t length);
int Fork();
int Execve(char *filename, char *argv[], char *envp[]);
int Dup2(int oldfd, int newfd);
int Wait(int *status);


int open_listenfd(int port);
int open_clientfd(char *hostname, int port);


int Rio_writen(int fd, void *usrbuf, size_t n);

#endif
