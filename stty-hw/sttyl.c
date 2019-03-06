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
#include	<sys/ioctl.h>
#include	"sttyl.h"

#define ERROR 1
#define C_OFFSET ('A' - 1)
#define ON	1
#define OFF 0

/*
 * ==========================
 *   TABLES -- to be moved
 * ==========================
 */

typedef struct flaginfo f_table;
typedef struct cflaginfo c_table;

f_table input_flags[] = {
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

f_table output_flags[] = {
	{ OPOST		,	"opost" },
//	{ ONLCR		,	"map NL to CR-NL"},
// 	{ OCRNL		,	"Map CR to NL on output" },
// 	{ ONOCR		,	"Don't output CR at column 0" },
// 	{ ONLRET	,	"Don't output CR" },
// 	{ OFILL		,	"Send fill characters for a delay" },
	{ 0			,	NULL }
};

f_table control_flags[] = {
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

f_table local_flags[] = {
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
	{ VEOF		,	"eof"} ,
	{ VEOL		,	"eol" },
	{ VERASE	,	"erase" },
	{ VINTR		,	"intr" },
	{ VKILL		,	"kill" },
//	{ VMIN		,	"min" },
	{ VQUIT		,	"quit" },
// 	{ VSTART	,	"start" },
// 	{ VSTOP		,	"stop" },
	{ VSUSP		,	"susp" },
//	{ VTIME		,	"time" },
	{ 0			,	NULL },
};

// struct flags all_flags[] = {
// 	output_flags, 
// 	control_flags,
// 	local_flags,
// 	input_flags
// };

// struct table = { f_table [] };
// f_table all_flags[] = {input_flags, output_flags, control_flags, local_flags};
struct table all_flags = {
	{ "input_flags", input_flags},
	{ "output_flags", output_flags },
	{ "control_flags", control_flags },
	{ "local_flags", local_flags }
};


void show_tty(struct termios *info);
void get_option(char *, struct termios *);
int change_char(char *, char *, struct termios *);
void show_charset(cc_t [], struct cflaginfo [], char *);
void show_flagset(int, f_table [], char *);
//void check_setting(int, f_table [], char *);
int check_setting(char *, int, struct termios *);
void fatal(char *, char *);
f_table * lookup(char *);
f_table * check_array(f_table[], char *);

/* FILE-SCOPE VARIABLES*/
static char *progname;			//used for error-reporting


/*
 *	main()
 *	Purpose: 
 *	 Method: 
 */
int main(int ac, char *av[])
{
	int i = 0;
	struct termios ttyinfo;
	get_settings(&ttyinfo);						//pull in current settings
	progname = *av++;							//initialize to program name

	if (ac == 1)								//no arguments
	{
		show_tty(&ttyinfo);						//just show default info
		return 0;
	}

	//go through arguments
	while(av[i])
	{
		//it is the erase or kill option, check next argument
		if(strcmp(av[i], "erase") == 0 || strcmp(av[i], "kill") == 0)
		{
			if(av[i + 1])
			{
				change_char(av[i], av[i + 1], &ttyinfo);
				i += 2;
			}
			else
			{
				fatal("missing argument to", av[i]);
			}
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

void fatal(char *err, char *arg)
{
	fprintf(stderr, "%s: %s `%s'\n", progname, err, arg);
	exit(1);
}

/*
 *	change_char()
 *	Purpose: check args are valid and update either "erase" or "kill" char
 *	  Input: 
 *	 Errors: 
 */
int change_char(char *command, char *value, struct termios *info)
{
	//value is not a single char
	if (strlen(value) > 1)
		fatal("invalid integer argument", value);
	
	if(strcmp(command, "erase") == 0)
		info->c_cc[VERASE] = value[0];				//update VERASE value
	else if(strcmp(command, "kill") == 0)
		info->c_cc[VKILL] = value[0];				//update VKILL value
	else
		fatal("invalid argument", command);			//not "erase" or "kill"
	
	return 0;
}




void get_option(char *option, struct termios *info)
{
	int status = ON;
	printf("BEFORE option is %s\n", option);
	
	if(option[0] == '-')
	{
		status = OFF;
		option++;
	}
	
	printf("AFTER option is %s\n", option);

	f_table *item = lookup(option);

/*	if (check_setting(option, status, info) != 0)
	{
		fprintf(stderr, "error with check_setting\n");
	}*/
	
	printf("RETURNED FROM lookup\n");
	printf("Status is %d\n", status);
		
/*	if(status == ON)
	{
		info->c_lflag |= ECHO;	//turn on bit
	}
	else
	{
		info->c_lflag &= ~ECHO;	//turn off bit
	}	
*/	
	
	if(strcmp(option, "echo") == 0)
		printf("current val is %d\n", (int) (info->c_lflag & ECHO));

	return;
}
/*
//void check_setting(int mode, f_table flags[], char *option)
int check_setting(char * option, int status, struct termios *info)
{
// 	show_flagset(info->c_iflag, input_flags, "iflags");		//input flags
// 	show_flagset(info->c_cflag, control_flags, "cflags");	//control flags
// 	show_flagset(info->c_lflag, local_flags, "lflags");		//local flags
// 	show_flagset(info->c_oflag, output_flags, "oflags");	//output flags

	int i;
	
	
	for(i = 0; flags[i].fl_value != 0; i++)
	{
		if(strcmp(option, flags[i].fl_name) == 0)
		{
//			printf("MATCHED option, %s\t", flags[i].fl_name);
		
			if (mode & flags[i].fl_value)
				printf("%s \n", flags[i].fl_name);
			else
				printf("-%s \n", flags[i].fl_name);
		}
		else
		{
//			printf("NOPE, %s\n", flags[i].fl_name);
		}
			
	}
}*/

f_table * lookup(char *option)
{
	f_table *info = malloc(sizeof(f_table));
	
	if(info == NULL)
	{
		fprintf(stderr, "error with malloc()ing memory\n");
		return NULL;
	}
	
	//get_flag(&info, option);
	info = check_array(input_flags, option);
	
	if(info == NULL)
		info = check_array(control_flags, option);
		
	if(info == NULL)
		info = check_array(local_flags, option);
	
	if(info == NULL)
		info = check_array(output_flags, option);
	
	if(info == NULL)
	{
		fatal("the option doesn't exist", option);
	}
	
	return info;
}

/*
void get_flag(f_table * store, char * option)
{
	
	if(store = check_array(input_flags, option) == 0)
		store = input_flags[];
	
	if(check_array(control_flags, option) == 0)
		store = control_flags[];
	
	if(check_array(local_flags, option) == 0)
		store = local_flags[];
	
	if(check_array(output_flags, option) == 0)
		store = output_flags[];
	
	if(store == NULL)
		fatal("invalid attribute", option);



		
	return;
}*/

f_table * check_array(f_table flags[], char *option)
{
	int i;
	
	for (i = 0; flags[i].fl_name != NULL; i++)
	{
		if(strcmp(flags[i].fl_name, option) == 0)
		{
			printf("index of option is %d\n", i);
			return &flags[i];
			//return i;
		}
	}
	
	return NULL;
}






/*
 *	show_flagset()
 *	Purpose: 
 *	  Input: 
 */
void show_flagset(int mode, f_table flags[], char *name)
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
		
	return;
}

/*
 *	show_charset()
 *	Purpose: 
 *	  Input: 
 */
void show_charset(cc_t info[], struct cflaginfo chars[], char *name)
{
//	printf("in show_charset, first element is %d\n", chars[10].c_value);
	int i;
	
	for(i = 0; chars[i].c_name != NULL; i++)
	{
		if(i == 0)
			printf("%s: ", name);
		
		cc_t value = info[chars[i].c_value];
		//printf("\nTEMP cc_t value is %d\t", value);
		
		//for unprintable values, use ^X notation, where X is value + offset
		if(value < 32)
			printf("%s = ^%c; ", chars[i].c_name, value + C_OFFSET);
		else if (value == 127)
			printf("%s = ^%c; ", chars[i].c_name, value - C_OFFSET);
		else if (value > 127)
			printf("%s = <undef>; ", chars[i].c_name);
		else
			printf("%s = %c; ", chars[i].c_name, value);
	}
	
	if (i > 0)
		printf("\n");
	
	return;
}

/*
 *	show_tty()
 *	Purpose: display the current settings for the tty
 *	  Input: info, the struct containing the setting information
 *	 Output: a collection of settings, separated by ';' and sorted by type
 *	 Errors: get_term_size() will exit(1) if it encounters an error
 */
void show_tty(struct termios *info)
{
	//get terminal size
	struct winsize w = get_term_size();

	// print info
	printf("speed %d baud; ", (int) cfgetospeed(info));		//baud speed		
	printf("rows %d; ", w.ws_row);							//rows
	printf("cols %d;\n", w.ws_col);							//cols
	
	show_charset(info->c_cc, char_flags, "cchars");			//characters
	
	show_flagset(info->c_iflag, input_flags, "iflags");		//input flags
	show_flagset(info->c_cflag, control_flags, "cflags");	//control flags
	show_flagset(info->c_lflag, local_flags, "lflags");		//local flags
	show_flagset(info->c_oflag, output_flags, "oflags");	//output flags

	return;
}




