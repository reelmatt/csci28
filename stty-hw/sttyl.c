/*
 * ==========================
 *   FILE: ./sttyl.c
 * ==========================
 * Purpose: Set a limited number of options for a terminal device interface.
 *
 * Outline: sttyl with no arguments will print the current values for options
 *			it knows about. Special characters you can change are erase and
 *			kill. Other attributes can be set (turned on) using the name, or
 *			unset (turned off) by adding a '-' before the attribute. See usage
 *			below for examples.
 *
 * Usage:	./sttyl							-- no options, prints current vals
 *			./sttyl -echo onlcr erase ^X	-- turns off echo, turns on onlcr
 *											   and sets the erase char to ^X
 *
 * Data structures: sttyl is a table-driven program. The tables can be found
 *		in tty_tables.c where there is an array of structs for each of the
 *		four flag types and one for the special character array. There is an
 *		additional table that stores the four flag tables with pointers to
 *		the tables and to the termios struct read when the program is run.
 */

/* INCLUDES */
#include	<stdio.h>
#include	<termios.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/ioctl.h>
#include	<ctype.h>
#include	"sttyl.h"

/* OPTION PROCESSING */
void get_option(char *);
struct cchars * valid_char_opt(char *);

/* DISPLAY INFO */
void show_tty(struct termios *info);
void show_charset(cc_t [], char *);
void show_flagset();

/* SETTING VALUES*/
void change_char(struct cchars *, char *, struct termios *);

/* HELPER FUNCTIONS */
void fatal(char *, char *);
int check_setting(char *, int, struct termios *);
tcflag_t * lookup(char *, struct flags **);
struct table_t * new_lookup(char *);
struct flags * check_array(struct flags[], char *);

/* FILE-SCOPE VARIABLES*/
static char *progname;			//used for error-reporting
static struct termios ttyinfo;	//store the terminal settings

struct table_t {tcflag_t flag; char *name; char *type; tcflag_t *mode; };
struct table_t table[] = {
	{ ICRNL	, "icrnl" , "iflag", &ttyinfo.c_iflag },		//input_flags
	{ OPOST	, "opost" , "oflag", &ttyinfo.c_oflag },		//output_flags
	{ HUPCL	, "hupcl" , "cflag", &ttyinfo.c_cflag },		//control_flags
	{ ISIG	, "isig"  , "lflag", &ttyinfo.c_lflag },		//local_flags
	{ ICANON, "icanon", "lflag", &ttyinfo.c_lflag },
	{ ECHO	, "echo"  , "lflag", &ttyinfo.c_lflag },
	{ ECHOE	, "echoe" , "lflag", &ttyinfo.c_lflag },
	{ ECHOK	, "echok" , "lflag", &ttyinfo.c_lflag },
	{ 0		, NULL	  , 0 },
};

/*
 *	main()
 *	 Method: Load the current termios settings and process command-line
 *			 arguments. If none, it prints out the current values. Otherwise,
 *			 check for invalid/missing arguments, update the values, and set
 *			 the termios struct accordingly.
 *	 Return: 0 on success, 1 on error. If there is an invalid/missing
 *			 argument, the corresponding helper function will exit 1.
 */
int main(int ac, char *av[])
{
//	struct termios ttyinfo;
	get_settings(&ttyinfo);							//pull in current settings
	progname = *av;									//init to program name

	if (ac == 1)									//no args, just progname
	{
		show_tty(&ttyinfo);							//show default info
		return 0;									//and stop
	}
	
	while(*++av)
	{
		struct cchars * c = valid_char_opt(*av);	//special-char option?
		
		if(c != NULL)								//it is
		{
			if(av[1])								//check next arg exists
			{
				change_char(c, av[1], &ttyinfo);	//change it, or fatal()
				av++;								//extra arg skip
			}
			else
				fatal("missing argument to", *av);	//no arg for special-char
		}
		else										//a different attribute
			get_option(*av);
	}

	return set_settings(&ttyinfo);					//0 on success, 1 on error
}

/*
 *	valid_char_opt()
 *	Purpose: Check to see if an argument matches one of the special chars
 *	  Input: arg, the option to check
 *	 Return: cchars struct that matches the arg; NULL if no match
 */
struct cchars * valid_char_opt(char * arg)
{
	int i;
	
	struct cchars * chars = get_chars();		//load the char_table
	
	for(i = 0; chars[i].c_name != NULL; i++)	//go through all char options
	{
		if(strcmp(arg, chars[i].c_name) == 0)	//if it matches arg requested
			return &chars[i];					//return ptr to that struct
	}

	return NULL;
}

/*
 *	fatal()
 *	Purpose: print message to stderr and exit
 *	  Input: err, the type of error that was encounters
 *			 arg, the value of the argument that caused a problem
 *	 Return: Exit with 1.
 */
void fatal(char *err, char *arg)
{
	fprintf(stderr, "%s: %s `%s'\n", progname, err, arg);
	exit(1);
}


/*
 *	change_char()
 *	Purpose: update a control char (i.e. "erase" or "kill")
 *	  Input: c, the struct containing the index to update
 *			 value, the command-line to arg containing the new char
 *			 info, the struct where the new value is placed
 *	 Errors: If the "value" argument is more than 1-char long, it is
 *			 invalid, so print error and exit 1.
 *	   Note: Bullet #2 in the assignment handout mentions the program is
 *			 not required to handle caret-letter input. If it did, this
 *			 is where it would be implemented.
 */
void change_char(struct cchars * c, char *value, struct termios *info)
{
	//value is not a single char
	if (strlen(value) > 1)
		fatal("invalid integer argument", value);

	//set the value
	info->c_cc[c->c_value] = value[0];
	
	return;
}

/*
 *	get_option()
 *	Purpose: Turn the given option on/off in the termios struct
 *	  Input: option, the argument to check and turn on/off
 *	 Return: 
 */
void get_option(char *option)
{
	int status = ON;
	char * original = option;					//"store" the original
	struct table_t * entry = NULL;				//place to put flag info

	
	if(option[0] == '-')						//check if a leading dash
	{
		status = OFF;							//will turn option off
		option++;								//trim dash from option
	}

	if ( (entry = new_lookup(option)) == NULL)	//lookup appropriate flag
		fatal("illegal option", original);		//couldn't find it, exit
		
	if(status == ON)
		*entry->mode |= entry->flag;			//turning on
	else
		*entry->mode &= ~entry->flag;			//turning off

	return;
}

/*
 *	lookup()
 *	Purpose: Find a given option in the defined tables (in tty_tables.c)
 *	  Input: option, the argument we are searching for
 *			 flag, where to store the struct that matched
 *	 Return: If a match is found, the struct is assigned to the "flag" from
 *			 input, and the corresponding tcflag_t is returned. Otherwise,
 *			 NULL is returned to indicate failure.
 */
struct table_t * new_lookup(char *option)
{
	int i;
	for (i = 0; table[i].name != NULL; i++)
	{
		if(strcmp(option, table[i].name) == 0)
			return &table[i];
	}
	
	return NULL;
}

/*
 *	show_flagset()
 *	Purpose: 
 *	  Input: 
 */
//void show_flagset(int mode, f_info flags[], char *name)
void show_flagset()
{
	char * type = NULL;
	
	int i;
	for(i = 0; table[i].name != NULL; i++)
	{
		if(type == NULL || strcmp(type, table[i].type) != 0)
		{
			type = table[i].type;			//switch to new flag type
			printf("\n%ss: ", type);		//add extra 's' to the type
		}
	
		if ((*table[i].mode & table[i].flag) == table[i].flag)
			printf("%s ", table[i].name);		//if ON, just print
		else
			printf("-%s ", table[i].name);		//if OFF, add '-'
	
	}
	
	if (i > 0)
		printf("\n");
		
	return;
}

/*
 *	show_charset()
 *	Purpose: 
 *	  Input: 
 *	   Note: See piazza post @171. For unprintable values, use ^X notation,
 *			 where X is value XORed with the CHAR_MASK, 64. In practice, this
 *			 adds 64 to values 0-31 and subtracts from value 127 (delete).
 */
void show_charset(cc_t info[], char *name)
{
	int i;
	struct cchars * chars = get_chars();
	
	for(i = 0; chars[i].c_name != NULL; i++)
	{
		cc_t value = info[chars[i].c_value];
		
		//If the first value, print a header
		if(i == 0)
			printf("%s: ", name);
		

		//Print the name and corresponding value, see "Note" above
		if (value == _POSIX_VDISABLE)
			printf("%s = <undef>; ", chars[i].c_name);
		else if(iscntrl(value))
			printf("%s = ^%c; ", chars[i].c_name, value ^ CHAR_MASK);
		else
			printf("%s = %c; ", chars[i].c_name, value);
	}
	
	//If values existed, add a newline
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
	//get terminal size, baud speed, and load tables
	struct winsize w = get_term_size();
	int baud = getbaud(cfgetospeed(info));
//	struct table * all = get_table();

	// print info
	printf("speed %d baud; ", baud);		//baud speed
	printf("rows %d; ", w.ws_row);			//rows
	printf("cols %d;\n", w.ws_col);			//cols
	show_charset(info->c_cc, "cchars");		//special characters
	show_flagset();							//current flag states

	return;
}




