#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include "sttyl.h"

/* copied from termfuncs.c from proj0 (more03.c) at beginning of class */
struct winsize get_term_size()
{
	struct winsize w;
	
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != 0)
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


int set_settings(struct termios *info)
{
	//from setecho-better.c, week 5 course files
	if ( tcsetattr( 0, TCSANOW, info ) == -1 )
	{
		perror("Setting attributes");
		return 1;
	}

	return 0;
}


/*
 *
 *Note: copied from showtty.c file from week5 lecture, with modifications
 */
int getbaud(int speed)
{
	switch(speed)
	{
		case B0:		return 0;		break;
		case B50:		return 50;		break;
		case B75:		return 75;		break;
		case B110:		return 110;		break;
		case B134:		return 134;		break;
		case B150:		return 150;		break;
		case B200:		return 200;		break;
		case B300:		return 300;		break;
		case B600:		return 600;		break;
		case B1200:		return 1200;	break;
		case B1800:		return 1800;	break;
		case B2400:		return 2400;	break;
		case B4800:		return 4800;	break;
		case B9600:		return 9600;	break;
		case B19200:	return 19200;	break;
		case B38400:	return 38400;	break;
		default:		return 0;		break;
	}
}
