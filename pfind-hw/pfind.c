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

	//process command-line args
	while (*av)
	{
		if (!path)							//no starting_path has been given
			get_path(*av, &path);			//fatal() if not valid
		else								//check if args are valid options
			get_option(av++, &name, &type);	//fatal() if not valid

		av++;
		ac++;
	}
	
	if (path)								//if path was specified
		searchdir(path, name, type);		//perform find there, with options
	else
		fprintf(stderr, "usage: pfind starting_path [-name ...] [-type ...]\n");
		
	return 0;
}

void get_path(char *val, char **path)
{
	if(val[0] != '-')
		*path = val;
	else
		fatal(progname, "paths must precede expression:", "");
	
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
	char *full_path = NULL;				//path to file
	DIR* current_dir;					//pointer to directory
	struct dirent *dp = NULL;			//pointer to "file"
	
/*	//get stat on initial entry
	if (lstat(dirname, info) == -1)
	{
		fprintf(stderr, "%s: `%s': %s\n", progname, dirname, strerror(errno)); //read err
		return;
	}


//	char *full_path = construct_path(dirname, );
	
	//check the first entry
	if (check_entry(findme, type, dirname, dirname, "", info->st_mode))
		printf("%s\n", dirname);
	
*/
	//dirname is not a dir, test for starting node as file
	if ( (current_dir = opendir(dirname)) == NULL )
	{
		//get stat on initial entry
		if (lstat(dirname, info) == -1)
		{
			fprintf(stderr, "%s: `%s': %s\n", progname, dirname, strerror(errno)); //read err
			return;
		}
		
		if (check_entry(findme, type, dirname, dirname, "", info->st_mode))
			printf("%s\n", dirname);
	}
	else
	{
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
		}	
	}
//there was an error
//fprintf(stderr, "%s: `%s': %s\n", progname, dirname, strerror(errno));
	
/*	
	//dirname is a dir, and one we have permission to open
	if ( (current_dir = opendir(dirname)) != NULL)
	{
		//iterate through all entries in the directory
		while( (dp = readdir(current_dir)) != NULL )
		{
			//turn parent/child into a single pathname
			full_path = construct_path(dirname, dp->d_name);

			if (lstat(full_path, info) == -1)
			{
				fprintf(stderr, "%s: `%s': %s\n", progname, full_path, strerror(errno));
				continue;
			}

			if (check_entry(findme, type, dp->d_name, full_path, info->st_mode))
				printf("%s\n", full_path);

			if ( recurse_directory(dp->d_name, info->st_mode) == YES )
				searchdir(full_path, findme, type);
			
			//free(full_path);
		}
	}
	//dirname is NOT a dir
	else if (errno == ENOTDIR)
	{	
		//full_path = construct_path(dirname, "");	//
//		printf("open not successful, trying as start file\n");
		if (lstat(dirname, info) == -1)			//see if dir node is file
		{
			fprintf(stderr, "%s: `%s': %s\n", progname, dirname, strerror(errno)); //nope
		}
		else if (check_entry(findme, type, dirname, dirname, info->st_mode))
		{
			if (S_ISDIR(info->st_mode))
				fprintf(stderr, "%s: `%s': %s\n", progname, dirname, strerror(errno));
			else
				printf("%s\n", dirname);				//it was a file, print
		}
		return;	
	}
	//otherwise, some other opendir error, just print error message
	else
	{
		fprintf(stderr, "%s: `%s': %s\n", progname, dirname, strerror(errno));
//		printf("there was some other error\n");
//		read_fatal(progname, dirname);
		return;
		
	}
	*/
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
	//if the file isn't a directory, also return NO
	if (! S_ISDIR(mode) || strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return NO;
		
	//shouldn't be processing the current or parent directory entries
// 	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
// 		return NO;
	
	return YES;
}

int check_entry(char *findme, int type, char *full_path, char *dirname, char *fname, mode_t mode)
{
	//printf("\t\tin check_entry, full_path is %s, dirname is %s and fname is %s\n", full_path, dirname, fname);
	//shouldn't be processing the current or parent directory entries
// 	if (strcmp(fname, "..") == 0)
// 		return NO;
// 	


	
	//see "else" condition below for handling "." directories

	//check if name is specified and filter if no match
	if(findme)
	{
		if(fnmatch(findme, fname, FNM_NOESCAPE) != 0)
			return NO;
	}
	
	//check if type is specified and filter if no match
	if(type != 0)
	{
		if(check_type(type, mode) == NO)
			return NO;
	}

	if (strcmp(fname, "..") == 0)
		return NO;
	
	if (strcmp(fname, ".") == 0 && strcmp(fname, dirname) != 0)
			return NO;

/*		
	if (findme && type != 0)						//both args specified
	{
		if (fnmatch(findme, fname, FNM_NOESCAPE) == 0)
			if(check_type(type, mode) == 1)
				return YES;
	}
	else if (findme)								//only "name" specified
	{
		if (fnmatch(findme, fname, FNM_NOESCAPE) == 0)
			return YES;
		
	}
	else if (type != 0)							//only "type" specified
	{
		if (strcmp(fname, ".") == 0 && strcmp(fname, dirname) != 0)
			return NO;
			
		if (check_type(type, mode) == 1)
			return YES;
	}
	else											//no findme or type, so YES
	{
//		printf("in check_entry, name is %s and path is %s\n", name, path);
//		printf("else in check_entry\t");



// 		if(strcmp(fname, dirname) != 0)
// 			return YES;

		if (strcmp(fname, ".") == 0 && strcmp(fname, path) != 0)
//			printf("returning no...\t%s\n", path);
			return NO;
		else
			return YES;
	}
*/
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
	int searchmode;
	
	switch (c) {
		case 'b':
			searchmode = S_IFBLK;
			break;
		case 'c':
			searchmode = S_IFCHR;
			break;
		case 'd':
			searchmode = S_IFDIR;
			break;		
		case 'f':
			searchmode = S_IFREG;
			break;		
		case 'l':
			searchmode = S_IFLNK;
			break;		
		case 'p':
			searchmode = S_IFIFO;
			break;		
		case 's':
			searchmode = S_IFSOCK;
			break;		
		default:
			fprintf(stderr, "%s: Unknown argument to -type: %c\n", progname, c);
			exit (1);
	}
	
	return searchmode;
}


int check_type(int type, mode_t item)
{
	if ( (S_IFMT & item) == type )
		return YES;
	else
		return NO;
}


