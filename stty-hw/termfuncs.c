#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "sttyl.h"

/* copied from termfuncs.c from proj0 (more03.c) at beginning of class */
// int get_term_size(int rows_cols[2])
// {
// 	struct winsize w;
// 	int rv;
// 	
// 	//original code used STDOUT_FILENO for fd arg -- why?
// 	rv = ioctl(0, TIOCGWINSZ, &w);
// 	if(rv == 0)
// 	{
// 		rows_cols[0] = w.ws_row;
// 		rows_cols[1] = w.ws_col;
// 	}
// 	
// 	return rv;
// }

struct winsize get_term_size()
{
	struct winsize w;
	
	if(ioctl(0, TIOCGWINSZ, &w) != 0)
	{
		fprintf(stderr, "could not get window size\n");
		exit(1);
	}
	
	return w;	
}


void get_settings(struct termios *info)
{
	if(isatty(0) == 1)
	{
//		printf("it IS a tty\n");
//		printf("name is %s\n", ttyname(0));
	}
	else
		printf("nope, not a tty");

	if ( tcgetattr(0, info) == -1 )
	{
		perror("cannot get tty info for stdin");
		exit(1);
	}
	
	return;
}

void set_settings(struct termios *info)
{
	//from setecho-better.c, week 5 course files
	if ( tcsetattr( 0, TCSANOW, info ) == -1 )
	{
		perror("Setting attributes");
		exit(1);
	}
	
	//printf("setting attributes SUCCESSFUL\n");
	return;
}