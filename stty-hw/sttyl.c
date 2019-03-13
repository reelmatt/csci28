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


void show_tty(struct termios *info);
void get_option(char *, struct termios *);
int change_char(char *, char *, struct termios *);
void show_charset(cc_t [], char *);
void show_flagset(tcflag_t *, char *);
//void show_flagset(int, f_info [], char *);
//void check_setting(int, f_table [], char *);
int check_setting(char *, int, struct termios *);
void fatal(char *, char *);
tcflag_t * lookup(char *, f_info **);
f_info * check_array(f_info[], char *);

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

	if (ac == 1)								//no arguments, just progname
	{
		show_tty(&ttyinfo);						//show default info
		return 0;								//and stop
	}

	//go through arguments
	while(av[i])
	{
		//it is a special-char option, check next argument
		if(strcmp(av[i], "erase") == 0 || strcmp(av[i], "kill") == 0)
		{
			if(av[i + 1])
			{
				change_char(av[i], av[i + 1], &ttyinfo);
				i++;
			}
			else
				fatal("missing argument to", av[i]);
		}
		//set a non-special-char attribute
		else
			get_option(av[i], &ttyinfo);
		
		i++;
	}

	set_settings(&ttyinfo);
	return 0;
}

void fatal(char *err, char *arg)
{
	fprintf(stderr, "%s: %s `%s'\n", progname, err, arg);
	exit(1);
}

//int check_char

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
	
	int i;
	c_info * chars = get_chars();
	
	for (i = 0; chars[i].c_name != NULL; i++)
	{
		if(strcmp(command, chars[i].c_name) == 0)
		{
			info->c_cc[chars[i].c_value] = value[0];
			return 0;
		}
	}
	
	fatal("invalid argument", command);			//not "erase" or "kill"
	
// 	if(strcmp(command, "erase") == 0)
// 		info->c_cc[VERASE] = value[0];				//update VERASE value
// 	else if(strcmp(command, "kill") == 0)
// 		info->c_cc[VKILL] = value[0];				//update VKILL value

	
	return 1;
}




void get_option(char *option, struct termios *info)
{
	int status = ON;
	
	if(option[0] == '-')
	{
		status = OFF;
		option++;
	}
	
	f_info *item = NULL;
	tcflag_t * mode = lookup(option, &item);

	//printf("RETURNED FROM LOOKUP\nMode is %lu\n", *mode);
	
	if(status == ON)				//turning on
	{
		//printf("mode is %lu and val is %lu\n", *mode, item->fl_value);
		*mode |= item->fl_value;
	}
	else							//turning off
	{
		//printf("mode is %lu and val is %lu\n", *mode, item->fl_value);
		*mode &= ~ item->fl_value;
	}
	

/*	if (check_setting(option, status, info) != 0)
	{
		fprintf(stderr, "error with check_setting\n");
	}*/
	

//	printf("Status is %d\n", status);
		
	

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

tcflag_t * lookup(char *option, f_info **flag)
{
// 	f_table *info = malloc(sizeof(f_table));
// 	
// 	if(info == NULL)
// 	{
// 		fprintf(stderr, "error with malloc()ing memory\n");
// 		return NULL;
// 	}
	
	struct table_entry * all = get_full_table();
	
	int i;
	for(i = 0; all[i].table_name != NULL; i++)
	{
		f_info *info = get_table(all[i].table_name);
		f_info *selected = check_array(info, option);
		
		if(selected != NULL)
		{
			//printf("Found it in table %s. Returning.\n", all[i].table_name);
			*flag = selected;
			return all[i].mode;
		}
		else
		{
			//printf("Not in table %s. Try again?\n", all[i].table_name);
		}
	}
	
	return 0;

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

f_info * check_array(f_info flags[], char *option)
{
	int i;
	
	for (i = 0; flags[i].fl_name != NULL; i++)
	{
		if(strcmp(flags[i].fl_name, option) == 0)
		{
//			printf("index of option is %d\n", i);
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
//void show_flagset(int mode, f_info flags[], char *name)
void show_flagset(tcflag_t * mode, char *kind)
{
// 	struct table_entry *table = get_full_table(kind);
// 	f_info * flags = table->table;
	
	
	f_info * flags = get_table(kind);
	//printf("Mode for %s is %lu\n", kind, *mode);
	int i;
	
	for (i = 0; flags[i].fl_value != 0; i++)
	{
		if(i == 0)
			printf("%s: ", kind);			

		if (*mode & flags[i].fl_value)			//if ON
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
 *	   Note: See piazza post @171. For unprintable values, use ^X notation,
 *			 where X is value XORed with the CHAR_MASK, 64. In practice, this
 *			 adds 64 to values 0-31 and subtracts from value 127 (delete).
 */
void show_charset(cc_t info[], char *name)
{
	int i;
	struct cflaginfo * chars = get_chars();
	
	for(i = 0; chars[i].c_name != NULL; i++)
	{
		cc_t value = info[chars[i].c_value];
		
		//If the first value, print a header
		if(i == 0)
			printf("%s: ", name);
		
		//Print the name and corresponding value, see "Note" above
		if(value < 32 || value == 127)
			printf("%s = ^%c; ", chars[i].c_name, value ^ CHAR_MASK);
		else if (value > 127)
			printf("%s = <undef>; ", chars[i].c_name);
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
	int i;

	//get terminal size, baud speed, and load tables
	struct winsize w = get_term_size();
	int baud = getbaud(cfgetospeed(info));
	struct table_entry * all = get_full_table();

	// print info
	printf("speed %d baud; ", baud);						//baud speed
	printf("rows %d; ", w.ws_row);							//rows
	printf("cols %d;\n", w.ws_col);							//cols
	
	//print special characters
	show_charset(info->c_cc, "cchars");
	
	//print all current options located in flag tables
	for (i = 0; all[i].table_name != NULL; i++)
	{
		show_flagset(all[i].mode, all[i].table_name);
	}

// 	show_flagset(info->c_iflag, get_table("input_flags"), "iflags");	//input flags
// 	show_flagset(info->c_cflag, get_table("control_flags"), "cflags");	//control flags
// 	show_flagset(info->c_lflag, get_table("local_flags"), "lflags");	//local flags
// 	show_flagset(info->c_oflag, get_table("output_flags"), "oflags");	//output flags

	return;
}


