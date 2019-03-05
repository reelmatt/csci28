/*
 * ==========================
 *   FILE: ./sttyl.c
 * ==========================
 * Purpose: Search directories and subdirectories for files matching criteria.
 *
 * Main features: as seen in the function signatures below, the main features
 *		of pfind can be broken into the main logic, memory allocation, option
 *		processing, and helper functions to output error messages.
 *
 * Outline: pfind recursively searches, depth-first, through directories and
 *		any subdirectories it encounters, starting with a provided path.
 *		Results are filtered according to user-specified "-name" and/or
 *		"-type" options.
 *
 *
 * Data structures: construct_path() will malloc() a block of memory to store
 *		the full path of the current directory entry returned by the call to
 *		readdir(). If it is a directory, this is passed through to be
 *		recursively searched, otherwise, the char * is freed.
 */

/* INCLUDES */
#include	<stdio.h>
#include	<termios.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	"sttyl.h"

#define ERROR 1
#define C_OFFSET ('A' - 1)

void show_tty(struct termios *info);
void get_settings(struct termios *);
void set_settings(struct termios *);
void get_option(char **, struct termios *);


/*
 *
 */
int main(int ac, char **av)
{
	struct termios ttyinfo;
	get_settings(&ttyinfo);

	//no arguments, just show default info
	if (ac == 1)
		show_tty(&ttyinfo);
	
	//go through arguments
	while(*++av)
	{
		if(strcmp(av[0], "erase") == 0 && av[1])
		{
			printf("change erase char to %c\n", av[1][0]);
			ttyinfo.c_cc[VERASE] = av[1][0];
			av += 2;
		}
		else if(strcmp(av[0], "kill") == 0 && av[1])
		{
			printf("change kill char to %c\n", av[1][0]);
			ttyinfo.c_cc[VKILL] = av[1][0];
			av += 2;
		}
		else
		{
			get_option(av, &ttyinfo);
		}
	}	
	
	set_settings(&ttyinfo);
	return 0;
}
/*
void update_setting(int mode, struct termios *info)
{
	info->c_cc[VERASE]
	return;
}*/

void get_option(char **av, struct termios *info)
{
	printf("option to change is %s\n", *av);
	
	
	
	if(strcmp(*av, "echo") == 0)
		printf("current val is %d\n", (info->c_lflag & ECHO));

	return;
}

void get_settings(struct termios *info)
{
	if(isatty(0) == 1)
	{
		printf("it IS a tty\n");
		printf("name is %s\n", ttyname(0));
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
	
	return;
}


void lookup()
{
	
	return;
}

void show_flagset(int mode, struct flaginfo flags[], char *name)
{
	int i;
	
	for (i = 0; flags[i].fl_value != 0; i++)
	{
		if(i == 0)
			printf("%s: ", name);			

		if (mode & flags[i].fl_value)
			printf("%s ", flags[i].fl_name);
		else
			printf("-%s ", flags[i].fl_name);
	}
	
	if (i > 0)
		printf("\n");
}

void show_tty(struct termios *info)
{
	//get terminal size
	int size[2];
	if (get_term_size(size) != 0)
		return;

	struct winsize w = get_term_alt();

	/* print info */
	printf("speed %lu baud; ", cfgetospeed(info));			//baud speed		
//	printf("rows %d; ", size[0]);							//rows
//	printf("cols %d;\n", size[1]);							//cols
	printf("rows %d; ", w.ws_row);
	printf("cols %d;\n", w.ws_col);

	printf("intr = ^%c; ", info->c_cc[VINTR] + C_OFFSET);	//intr
	printf("erase = ^%c; ", info->c_cc[VERASE] + C_OFFSET);	//erase
	printf("kill = ^%c; ", info->c_cc[VKILL] + C_OFFSET);	//kill
	printf("start = ^%c; ", info->c_cc[VSTART] + C_OFFSET);	//start
	printf("stop = ^%c;\n", info->c_cc[VSTOP] + C_OFFSET);	//stop
	
	show_flagset(info->c_iflag, &input_flags, "iflags");	//input flags
	show_flagset(info->c_cflag, &control_flags, "cflags");	//control flags
	show_flagset(info->c_lflag, &local_flags, "lflags");	//local flags
	show_flagset(info->c_oflag, &output_flags, "oflags");	//output flags
	
	return;
}

