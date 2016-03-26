#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void unix_error(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}
