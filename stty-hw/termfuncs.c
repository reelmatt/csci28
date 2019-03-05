#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

/* copied from termfuncs.c from proj0 (more03.c) at beginning of class */
int get_term_size(int rows_cols[2])
{
	struct winsize w;
	int rv;
	
	//original code used STDOUT_FILENO for fd arg -- why?
	rv = ioctl(0, TIOCGWINSZ, &w);
	if(rv == 0)
	{
		rows_cols[0] = w.ws_row;
		rows_cols[1] = w.ws_col;
	}
	
	return rv;
}