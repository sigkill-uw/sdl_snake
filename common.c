#define _COMMON_C_

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void die(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
