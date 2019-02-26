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

void usage_fatal()
{
	fprintf(stderr, "usage: pfind starting_path [-name ...] [-type ...]\n");
	exit(1);
}

void read_fatal(char *myname, char *path)
{
	//example -- ./pfind: `/tmp/pft.IO8Et0': Permission denied
	fprintf(stderr, "%s: `%s': ", myname, path);
	perror("");
//	fprintf(stderr, "\n");
}

void type_fatal(char *myname, char *type)
{
	//example -- ./pfind: missing argument to `-name'
	fprintf(stderr, "%s: missing argument to `%s'\n", myname, type);
	exit(1);
}