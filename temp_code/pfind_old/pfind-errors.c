#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pfind.h"



void fatal(char *myname, char *s1, char *s2)
{
	fprintf(stderr, "%s: `%s' ", myname, s1);
	
	if (strcmp(s2, "") != 0)
		perror(s2);
	else
		fprintf(stderr, "\n");
	
	exit(1);
}
