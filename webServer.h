#ifndef __WEB_SERVER_H
#define __WEB_SERVER_H

#define __DEBUG

#include "rio.h"

#define MAXLINE 1024
#define MAXBUF 8092

void clienterror(int fd, char *cause, char *errnum,
				char *shortmsg, char *longmsg);
void get_filetype(char *filename, char *filetype);
void read_request_hdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *args);
void server_static(int sockfd, char *filename, size_t fsize);
void server_dynamic(int sockfd, char *filename, char *cgiargs);
static char *Index(char *uri, char ch);

#endif
