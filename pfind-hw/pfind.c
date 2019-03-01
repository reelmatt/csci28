/*
 * ==========================
 *   FILE: ./pfind.c
 * ==========================
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
 * Method: Process command-line arguments, if any, and then call get_log()
 *		   to open the lastlog file (LLOG_FILE by default), with corresponding
 * 		   args, NULL if not specified.
 * Return: 0 on success, -1 on close() error, exits 1 and prints message to
 *		   stderr on other failures (see corresponding functions).
 *   Note: The while loop will cycle through options, any/all of -u, -t, or -f.
 *		   If it is not a valid option, fatal() is called and program exits.
 *		   get_option is only called when there is at least one more arg left
 *		   in addition to the '-' option, the (i+1) < ac part in the if case.
 */
int main (int ac, char **av)
{
	progname = *av++;							//initialize to program name
	
	//variables set to default values for user options
	char *path = NULL;
	char *name = NULL;
	int type = 0;

	//syntax error, see above
	if(ac > 6)
		fprintf(stderr, "usage: pfind starting_path [-name ...] [-type ...]\n");

	//process command-line args
	while (*av)
	{
		if (!path)							//no starting_path has been given
			get_path(av, &path, &name, &type);			//fatal() if not valid
		else								//check if args are valid options
			get_option(av++, &name, &type);	//fatal() if not valid

		av++;
	}
	
	if (path)								//if path was specified
		searchdir(path, name, type);		//perform find there, with options
	else
		fprintf(stderr, "usage: pfind starting_path [-name ...] [-type ...]\n");
		
	return 0;
}

void get_path(char **val, char **path, char **name, int *type)
{
	
	if(*val[0] != '-')			//arg does not begin with option specifier
		*path = *val;
	else						//arg DOES begin with option specifier '-'
	{
		
		while(*val && *val[0] == '-')
		{
			get_option(val, name, type);
			val += 2;
		}
		
		if(*val)
			fprintf(stderr, "%s: paths must precede expression: %s\n", progname, *val);
		else
			fprintf(stderr, "usage: pfind starting_path [-name ...] [-type ...]\n");

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
//	printf("\n\nstarting search...\n");
	struct stat *info = new_stat();		//file info
//	char *full_path = NULL;				//path to file
	DIR* current_dir;					//pointer to directory
	struct dirent *dp = NULL;			//pointer to "file"
	
	//dirname is not a dir, test for starting node as file
	if ( (current_dir = opendir(dirname)) == NULL )
	{
		process_file(dirname, findme, type, info);
/*		//get stat on initial entry
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
			*/
	}
	else
	{
		process_dir(dirname, findme, type, current_dir, dp, info);
	/*
		//read through entries
		while( (dp = readdir(current_dir)) != NULL )
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
		}	*/
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

int check_entry(char *findme, int type, char *full_path, char *dirname, char *fname, mode_t mode)
{
	//check if name is specified and filter if no match
	if(findme && fnmatch(findme, fname, FNM_NOESCAPE) != 0)
		return NO;

	//check if type is specified and filter if no match
	if(type != 0 && check_type(type, mode) == NO)
		return NO;

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
		if(value)
			*name = value;
		else
			type_fatal(progname, "-name");
	}
	else if (strcmp(option, "-type") == 0)
	{
		if (value)
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
 * Purpose: initial check to see if -type is supported
 *   Input: c, the char to check
 *  Return: c, the same char pass in, if one of 'bcdflps'. Otherwise,
 *			error is printed and program exits.
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


int check_type(int type, mode_t item)
{
	if ( (S_IFMT & item) == type )
		return YES;
	else
		return NO;
}


