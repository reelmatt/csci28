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

/*
 * ==========================
 *   TABLES -- to be moved
 * ==========================
 */

struct flaginfo input_flags[] = {
// 	{ IGNBRK	,	"Ignore BREAK condition on input" },
// 	{ BRKINT	,	"Signal interrupt on break" },
// 	{ IGNPAR	,	"Ignore chars with parity errors" },
// 	{ PARMRK	,	"Mark parity errors" },
// 	{ INPCK		,	"Enable input parity check" },
// 	{ ISTRIP	,	"Strip character" },
// 	{ INLCR		,	"Map NL to CR on input"},
// 	{ IGNCR		,	"Ignore CR"},
	{ ICRNL		,	"icrnl" },
// 	{ IXON		,	"Enable start/stop output control" },
// 	{ IXOFF		,	"Enable start/stop input control"},
	{ 0			,	NULL}
};

// 
// struct flaginfo get_input_flags()[]
// {
// 	return input_flags;
// }

struct flaginfo output_flags[] = {
	{ OPOST		,	"opost" },
//	{ ONLCR		,	"map NL to CR-NL"},
// 	{ OCRNL		,	"Map CR to NL on output" },
// 	{ ONOCR		,	"Don't output CR at column 0" },
// 	{ ONLRET	,	"Don't output CR" },
// 	{ OFILL		,	"Send fill characters for a delay" },
	{ 0			,	NULL }
};


struct flaginfo control_flags[] = {
// 	{ CSIZE		,	"Character size mask" },
// 	{ CSTOPB	,	"Set two stop bits, rather than one" },
// 	{ CREAD		,	"Enable receiver" },
// 	{ PARENB	,	"Parity enable" },
// 	{ PARODD	,	"Odd parity, else even" },
	{ HUPCL		,	"hupcl" },
// 	{ CLOCAL	,	"Ignore modem status lines" },
//	{ CIGNORE	,	"Ignore control flags" },
	{ 0			,	NULL }
};


struct flaginfo local_flags[] = {
	{ ISIG		,	"isig" },
	{ ICANON	,	"icanon" },
	{ ECHO		,	"echo" },
	{ ECHOE		,	"echoe" },
	{ ECHOK		,	"echok" },
// 	{ ECHONL	,	"Echo the NL character" },
// 	{ NOFLSH	,	"Disable flushing the input and output queues" },
// 	{ TOSTOP	,	"Send the SIGTTOU signal" },
	{ 0			,	NULL }
};


struct cflaginfo char_flags[] = {
	{ VEOF		,	"EOF"} ,
	{ VEOL		,	"EOL" },
	{ VERASE	,	"Erase char" },
	{ VINTR		,	"Interrupt" },
	{ VKILL		,	"Kill char" },
	{ VMIN		,	"Minimum number of chars for noncanon read" },
	{ VQUIT		,	"Quit" },
	{ VSTART	,	"Start character" },
	{ VSTOP		,	"Stop character" },
	{ VSUSP		,	"suspend char" },
	{ VTIME		,	"Timeout in deciseconds for noncanon" },
	{ 0			,	NULL },
};





void show_tty(struct termios *info);
void get_settings(struct termios *);
void set_settings(struct termios *);
void get_option(char *, struct termios *);
int change_char(char *, char *, struct termios *);

/*
 *
 */
int main(int ac, char *av[])
{
	struct termios ttyinfo;
	get_settings(&ttyinfo);
	int i = 1;

	//no arguments, just show default info
	if (ac == 1)
	{
		show_tty(&ttyinfo);
		return 0;
	}

	//go through arguments
	while(av[i])
	{
		if((strcmp(av[i], "erase") == 0 || strcmp(av[i], "kill") == 0) && av[i + 1])
		{
			change_char(av[i], av[i + 1], &ttyinfo);
			i += 2;
		}
		else
		{
			get_option(av[i], &ttyinfo);
			i++;
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

int change_char(char *command, char *value, struct termios *info)
{
	if (strlen(value) > 1)
	{
		fprintf(stderr, "sttyl: invalid integer argument `%s'\n", value);
		exit(1);
	}
	
	if(strcmp(command, "erase") == 0)
	{
		printf("value is %s and temp is %d\n", value, value[0]);
		info->c_cc[VERASE] = value[0];
	}
	else if(strcmp(command, "kill") == 0)
	{
		printf("value is %s and temp is %d\n", value, value[0]);
		info->c_cc[VKILL] = value[0];
	}
	else
	{
		fprintf(stderr, "sttyl: invalid argument `%s'\n", command);
		exit(1);
	}
	
	return 0;
}

void get_option(char *option, struct termios *info)
{
	printf("option to change is %s\n", option);
	
	
	
	if(strcmp(option, "echo") == 0)
		printf("current val is %lu\n", (info->c_lflag & ECHO));

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
	
	printf("setting attributes SUCCESSFUL\n");
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

//	struct winsize w = get_term_alt();

	/* print info */
	printf("speed %lu baud; ", cfgetospeed(info));			//baud speed		
	printf("rows %d; ", size[0]);							//rows
	printf("cols %d;\n", size[1]);							//cols
//	printf("rows %d; ", w.ws_row);
//	printf("cols %d;\n", w.ws_col);

	printf("intr = ^%c; ", info->c_cc[VINTR] + C_OFFSET);	//intr
	printf("intr = %d;\n", info->c_cc[VINTR]);
	
	printf("erase = ^%c; ", info->c_cc[VERASE] + C_OFFSET);	//erase	
	printf("erase = %d;\n", info->c_cc[VERASE]);	

	printf("kill = ^%c; ", info->c_cc[VKILL] + C_OFFSET);	//kill
	printf("kill = %d;\n", info->c_cc[VKILL]);	

	printf("start = ^%c; ", info->c_cc[VSTART] + C_OFFSET);	//start
	printf("start = %d;\n", info->c_cc[VSTART]);	

	printf("stop = ^%c; ", info->c_cc[VSTOP] + C_OFFSET);	//stop
	printf("stop = %d;\n", info->c_cc[VSTOP]);
	
	show_flagset(info->c_iflag, input_flags, "iflags");	//input flags
	show_flagset(info->c_cflag, control_flags, "cflags");	//control flags
	show_flagset(info->c_lflag, local_flags, "lflags");	//local flags
	show_flagset(info->c_oflag, output_flags, "oflags");	//output flags
	
	return;
}











