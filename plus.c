#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 200

int main(void)
{
	char *buf, *p;
	char arg1[MAXLINE], arg2[MAXLINE];
	int n1, n2;

	//取得对应环境变量的值（即参数值）
	if((buf=getenv("QUERY_STRING")) != NULL)
	{
		p = strchr(buf, '&');
		*p = '\0';
		strcpy(arg1, buf);
		strcpy(arg2, p+1);
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}
	else
		n1 = n2 = 0;
	
	char content[MAXLINE];
	sprintf(content, "Welcome to add.com: ");
	sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
	sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1+n2);
	sprintf(content, "%sThanks for visiting!\r\n", content);

	printf("Content-length: %ld\r\n", strlen(content));
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);

	exit(0);
}
