/*
 * ==========================
 *   FILE: ./pfind.c
 * ==========================
 * Purpose: Search directories and subdirectories for files matching criteria.
 *
 *
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fnmatch.h>
#include <libgen.h>
#include "pfind.h"

void process_file(char *, char *, int, struct stat *);
void process_dir(char *, char *, int, DIR *, struct dirent *, struct stat *);

static char *progname;	//used in pfind-error.c functions
/*
 * main()
 *  Method: Process command-line arguments, if any, and then call searchdir()
 *			to recursively search based on the starting path provided. If no
 *			path is provided, output error message with usage.
 *  Return: 0 on success, exits 1 and prints message to stderr on other
 *			failures (see corresponding functions for more info).
 *    Note: Options are processed with the help of two functions, get_path()
 *			and get_option(). The program exits if more than six command line
 *			arguments exist, as that would be a syntax error (1 program name,
 *			1 start path, max 2 args for -name, and max 2 args for -type.)
 *
 *			On invalid or missing arguments, these functions will print an
 *			error and exit(1). Option processing is done by going through
 *			char **av. On path processing, av is incremented once (av++)
 *			after the if/else block as the path is one argument. For options,
 *			av is incremented twice, once in get_option to cycle past the
 *			"-option" flag, and the av++ after the if/else block to iterate
 *			past the value for the option.
 */
int main (int ac, char **av)
{
	//variables set to default values for user options
	char *path = NULL;
	char *name = NULL;
	int type = 0;

	progname = *av++;							//initialize to program name
	
	if(ac > 6)
		syntax_fatal();							//syntax error, see above

	while (*av)									//process command-line args
	{
		if (!path)								//no starting_path given
			get_path(av, &path, &name, &type);	//fatal() if not valid
		else									//check args are valid options
			get_option(av++, &name, &type);		//fatal() if not valid

		av++;
	}
	
	if (path)									//if path was specified
		searchdir(path, name, type);			//perform find there
	else
		syntax_fatal();							//otherwise, syntax error
		
	return 0;
}

/*
 *	get_path()
 *	Purpose: Test the command line argument to see if it is a valid path.
 *	  Input: args, the array of command line arguments
 *			 path, variable to store the specified path in
 *			 name, variable to use as placeholder for out-of-order option
 *			 type, variable to use as placeholder for out-of-order option
 *	 Return: Prints message to stderr and exit(1) on out of order options
 *			 or invalid options.
 *	 Method: 
 */
void get_path(char **args, char **path, char **name, int *type)
{
	//arg DOES NOT begin with option specifier
	if(*args[0] != '-')
	{
		*path = *args;
	}
	//arg DOES begin with option specifier '-'
	else
	{
		//process options and arguments first, a la 'find'
		while(*args && *args[0] == '-')
		{
			get_option(args, name, type);
			args += 2;
		}
		
		//if there are still argument left, it is a start_path
		if(*args)
		{
			fprintf(stderr, "%s: paths must precede expression: %s\n", progname, *args);
		}
		//otherwise, general syntax error
		else
		{
			syntax_fatal();
		}

		exit(1);
	}
	
	return;
}

/*
 * searchdir()
 * Purpose: Recursively search a directory, filtering output based on
 *			optional findme and type parameters.
 *   Input: dirname, path of the current directory to search
 * 			findme, the pattern to look for/match against
 * 			type, the kind of file to search for
 *  Output: The path/filename for all matched entries. When findme and type
 *			are not specified, all entries are printed to stdout.
 * 
 */
void searchdir(char *dirname, char *findme, int type)
{

	struct stat info;					//file info
//	struct stat *info = new_stat();		//file info
//	char *full_path = NULL;				//path to file
	DIR* current_dir;					//pointer to directory
	struct dirent *dp = NULL;			//pointer to "file"
	
	//dirname is not a dir, test for starting node as file
	if ( (current_dir = opendir(dirname)) == NULL )
	{
		process_file(dirname, findme, type, &info);
	}
	else
	{
		process_dir(dirname, findme, type, current_dir, dp, &info);
	}

	//printf("about to free things...\n");
/*	
	free(full_path);
	//printf("freed full_path\n");
	free(info);
	//printf("freed info\n");
	closedir(current_dir);
	//printf("closed the dir\n");*/
	return;
}

/*
 *	process_file()
 *	Purpose: 
 *	  Input: 
 *	 Return: 
 */
void process_file(char *dirname, char *findme, int type, struct stat *info)
{
	//get stat on initial entry
	if (lstat(dirname, info) == -1)
	{
		fprintf(stderr, "%s: `%s': %s\n", progname, dirname, strerror(errno)); //read err
		return;
	}
	
	//check to see if it was a dir, just didn't have permissions
	if(S_ISDIR(info->st_mode))
	{
		fprintf(stderr, "%s: `%s': %s\n", progname, dirname, strerror(errno));
		return;
	}

	//filter start path/file according to criteria
	if (check_entry(findme, type, dirname, dirname, dirname, info->st_mode))
		printf("%s\n", dirname);
}

/*
 *	process_dir()
 *	Purpose: 
 *	  Input: 
 *	 Return: 
 */
void 
process_dir(char *dirname, char *findme, int type, DIR *search,
			struct dirent *dp, struct stat *info)
{
	char *full_path = NULL;
	
	//read through entries
	while( (dp = readdir(search)) != NULL )
	{
		//turn parent/child into a single pathname
		full_path = construct_path(dirname, dp->d_name);

		if (lstat(full_path, info) == -1)
		{
			fprintf(stderr, "%s: `%s': %s\n", progname, full_path, strerror(errno));
			continue;
		}

		if (check_entry(findme, type, full_path, dirname, dp->d_name, info->st_mode))
			printf("%s\n", full_path);

		if ( recurse_directory(dp->d_name, info->st_mode) == YES )
			searchdir(full_path, findme, type);
		
		//free(full_path);
	}	

}

/*
 * recurse_directory()
 * Purpose: check if the given directory entry is one we need to recurse
 *   Input: name, the current entry in the directory
 * 			mode, the current mode/type of file
 *  Return: NO, the file mode is not a directory. Or, the file is a directory
 * 				but it is either the current, ".", or parent, "..", entry.
 *			YES, the entry is a subdirectory that we should recursively search
 */
int recurse_directory(char *name, mode_t mode)
{
	//if the file isn't a directory, the current ".", or parent ".." dir
	if (! S_ISDIR(mode) || strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return NO;
	
	//all other cases, the file is a subdirectory to recursively search
	return YES;
}

/*
 *	check_entry()
 *	Purpose: 
 *	  Input: 
 *	 Return: 
 */
int check_entry(char *findme, int type, char *full_path, char *dirname, char *fname, mode_t mode)
{
	//check if name is specified and filter if no match
	if(findme && fnmatch(findme, fname, FNM_NOESCAPE) != 0)
		return NO;
		
	//check if type is specified and filter if no match
	if( (type != 0) && ((S_IFMT & mode) != type) )
		return NO;

/*
	//check if type is specified and filter if no match
	if(type != 0 && check_type(type, mode) == NO)
		return NO;
*/
//	if (strcmp(fname, dirname) != 0 )

	if (strcmp(fname, "..") == 0)
		return NO;
	
	if (strcmp(fname, ".") == 0 && strcmp(fname, dirname) != 0)
			return NO;

	return YES;	
}


/*
 *	get_option()
 *	Purpose: process command line options
 *	  Input: args, the array pointer to command-line arguments
 *			 name, pointer to store specified user
 *			 type, pointer to store specified file type
 *	 Return: None. This function passes through pointers from main
 *			 to store the variables.
 *	 Errors: Each criterion can only appear once. If the name or type
 *			 variables have been initialized, 
 *	 Errors: For the -u and -t options, parsing functions are called to
 *			 determine if the input is valid. extract_user() obtains the
 *			 passwd entry, or exits if not found/invalid. parse_time()
 *			 changes the text input into a number, or exits if not valid.
 *	  Notes: If there is an invalid option (not -utf), fatal is called
 *			 to output a message to stderr and exit with a non-zero status.
 *			 See also, errors above for invalid input.
 */
void get_option(char **args, char **name, int *type)
{
	char *option = *args++;				//store option, then point to next arg
	char *value = *args;				//store value for option (if any)
	
	if (strcmp(option, "-name") == 0)
	{
		if( value && (*name == NULL) )
			*name = value;
		else
			type_fatal(progname, "-name");
	}
	else if (strcmp(option, "-type") == 0)
	{
		if (value && *type == 0)
			*type = get_type(value[0]);
		else
			type_fatal(progname, "-type");
	}
	else
	{
		fprintf(stderr, "%s: unknown predicate `%s'\n", progname, option);
		exit(1);
	}

	return;
}

/*
 * get_type()
 * Purpose: check to see if -type is supported, and if so, provide bitmask
 *   Input: c, the char to check
 *  Return: the bitmask associated with the -type specified. If -type is
 *			not a valid option, print message to stderr and exit.
 * Options: the following options are allowed, as per <sys/stat.h>, and
 *			the 'find' command.
 *				b	block special
 *				c	character special
 *				d	directory
 *				f	regular file
 *				l	symbolic link
 *				p	FIFO
 *				s	socket
 */
int get_type(char c)
{
	switch (c) {
		case 'b':
			return S_IFBLK;
		case 'c':
			return S_IFCHR;
		case 'd':
			return S_IFDIR;	
		case 'f':
			return S_IFREG;	
		case 'l':
			return S_IFLNK;
		case 'p':
			return S_IFIFO;
		case 's':
			return S_IFSOCK;
		default:
			fprintf(stderr, "%s: Unknown argument to -type: %c\n", progname, c);
			exit (1);
	}
}

/*
 *	construct_path()
 *	Purpose: 
 *	  Input: 
 *	 Return: 
 */
char * construct_path(char *parent, char *child)
{
	char *newstr = malloc(PATHLEN);
	
	//Get memory for newstr and check
	if (newstr == NULL)
		fatal("", "memory error: not enough memory to create new string\n", "");
	
	if (strcmp(parent, child) == 0 || strcmp(parent, "") == 0)
		snprintf(newstr, PATHLEN, "%s", parent);
	else
		snprintf(newstr, PATHLEN, "%s/%s", parent, child);
	
	return newstr;
}

/*
 * new_stat()
 * Purpose: allocate memory for a stat struct, and check for errors
 *  Return: a pointer to the newly allocated stat struct
 *  Errors: if malloc fails, call fatal() to quit the program with an
 *			error message.
 */
struct stat * new_stat()
{
	struct stat *new_stat = malloc(sizeof(struct stat));
	
	if (new_stat == NULL)
		fatal("", "memory error: could not allocate a stat struct\n", "");
	
	return new_stat;
}
