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
#include <fnmatch.h>

#define NO 				0
#define YES 			1
#define PATHLEN			255


/*
	 b       block special
	 c       character special
	 d       directory
	 f       regular file
	 l       symbolic link
	 p       FIFO
	 s       socket
 */
char get_type(char);
void get_option(char *, char *, char **, char *);
void searchdir(char *, char *, char);
char * construct_path(char *, char *);
void fatal(char *);
void get_path(char *, char **);
struct stat * new_stat();
int check_type(char c, mode_t item);
int recurse_directory(char *, mode_t);
int check_entry(char *, char, char *, char *, mode_t);
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
int main (int ac, char *av[])
{
	int i = 1;

	//initialize variables to default values, changes with user options
	char *path = NULL;
	char *name = NULL;
	char type = '\0';

	//see Note section above for more on option processing
	while (i < ac)
	{
		if (!path)
		{
			get_path(av[i], &path);
		}
		else if(av[i][0] == '-' && (i + 1) < ac)
		{
			get_option(av[i], av[i + 1], &name, &type);
			i++;
		}
		else
		{
			fatal("usage: pfind starting_path [-name ...] [-type ...]");
		}

		i++;
	}
	
//	printf("passed in vars are %s, %s, %c\n", path, name, type);
	
	if (path)
		searchdir(path, name, type);
	else
		fatal("usage: pfind starting_path [-name ...] [-type ...]");

	return 0;
}

void get_path(char *val, char **path)
{
	if(val[0] != '-')
		*path = val;
	else
		fatal("not a valid path");
	
	return;
}

void fatal(char *s1)
{
	fprintf(stderr, "%s\n", s1);
	exit(1);
}

char * construct_path(char *parent, char *child)
{
	char *newstr = malloc(PATHLEN);
	
	//Get memory for newstr and check
	if (newstr == NULL)
		fatal("memory error: not enough memory to create new string\n");
	
	if (strcmp(parent, child) == 0)
		snprintf(newstr, PATHLEN, "%s", parent);
	else
		snprintf(newstr, PATHLEN, "%s/%s", parent, child);
	
	return newstr;
}

struct stat * new_stat()
{
	struct stat *new_stat = malloc(sizeof(struct stat));
	
	if (new_stat == NULL)
		fatal("memory error: could not allocate a stat struct\n");
	
	return new_stat;
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
void searchdir(char *dirname, char *findme, char type)
{
	//printf("passed in vars are %s, %s, %c\n", dirname, findme, type);
	
	DIR* current_dir;
	struct dirent *dp = NULL;
	struct stat *info = new_stat();
	
	//open the directory, exit on error with message
	if ( (current_dir = opendir(dirname)) == NULL)
	{
		perror(dirname);
		exit(1);
	}
	
	//iterate through all entries in the directory
	while( (dp = readdir(current_dir)) != NULL )
	{
		char *full_path = construct_path(dirname, dp->d_name);

		//printf("in while loop, what is full_path? - %s\n", full_path);
		
		if (lstat(full_path, info) == -1)
		{
			perror(full_path);
			continue;
		}

		if (check_entry(findme, type, dp->d_name, full_path, info->st_mode))
			printf("%s\n", full_path);
		
/*		//if either/both options specified, check against filter
		if (findme || type != '\0')
		{
			if (fnmatch(findme, dp->d_name, FNM_PERIOD) == 0)
				printf("findme is %s and full_path is %s\n", findme, full_path);
			else if (check_type(type, info->st_mode))
				printf("type is %c and full_path is %s\n", c, full_path);
			else
				printf("does not match\n");
			//printf("type is %c and check is %d\n", type, check_type(type, info->st_mode));
		}
		else
		{
			printf("%s\t\tno options specified...\n", full_path);
		}*/

		if ( recurse_directory(dp->d_name, info->st_mode) == YES )
			searchdir(full_path, findme, type);
		
		
/*		if (S_ISDIR(info->st_mode))
		{
			//don't process the current or parent directory entries
			if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
				continue;
			
			
			searchdir(full_path, findme, type);
		}*/
/*		else
		{
			printf("%s\t%hu\n", full_path, info->st_mode);
			//printf("full_path is %s\tinode is %llu\n", full_path, info->st_ino);
		}
*/		
		free(full_path);
	}
	
	
	closedir(current_dir);
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
	if (! S_ISDIR(mode) )
		return NO;
		
	//shouldn't be processing the current or parent directory entries
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return NO;
	
	return YES;
}

int check_entry(char *findme, char type, char *name, char *path, mode_t mode)
{
	//shouldn't be processing the current or parent directory entries
	if (strcmp(name, "..") == 0)
		return NO;
	
	if (strcmp(name, ".") == 0)
	{
		if (strcmp(name, path) == 0)
			return YES;
		else
			return NO;
	}
		
	//both are specified, check both
	if (findme && type != '\0')
	{
		if (fnmatch(findme, path, FNM_PERIOD) == 0)
			if(check_type(type, mode) == 1)
				return YES;
	}
	//only findme specified
	else if (findme)
	{
		if (fnmatch(findme, path, FNM_PERIOD) == 0)
			return YES;
	}
	else if (type != '\0')
	{
		if (check_type(type, mode) == 1)
			return YES;
	}
	else
	{
		return YES;		//no findme, type specified, so YES
	}

	return NO;	
//	return YES;
}


/*
 *	get_option()
 *	Purpose: process command line options
 *	  Input: opt, the char following the '-' flag
 *			 value, the argument following the [-utf] flag
 *			 user, pointer to store specified user
 *			 days, pointer to store specified time restriction
 *			 file, pointer to store specified file
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
void get_option(char *opt, char *val, char **name, char *type)
{
	if (strcmp(opt, "-name") == 0)
		*name = val;
	else if (strcmp(opt, "-type") == 0)
		*type = get_type(val[0]);
	else
		printf("else condition, get_option\n");

	return;
}

char get_type(char c)
{
	switch (c) {
		case 'b':
		case 'c':
		case 'd':
		case 'f':
		case 'l':
		case 'p':
		case 's':
			break;
		default:
			fprintf(stderr, "find -type: %c: unknown type.\n", c);
			exit (1);
	}
	
	return c;
}


int check_type(char c, mode_t item)
{
	int rv = 0;
	
	switch(c) {
		case 'b':
//			printf("case %c\t", c);
			rv = S_ISBLK(item);
			break;
		case 'c':
//			printf("case %c\t", c);
			rv = S_ISCHR(item);
			break;
		case 'd':
//			printf("case %c\t", c);
			rv = S_ISDIR(item);
			break;
		case 'f':
//			printf("case %c\t", c);
			rv = S_ISREG(item);
			break;
		case 'l':
//			printf("case %c\t", c);
			rv = S_ISLNK(item);
			break;
		case 'p':
//			printf("case %c\t", c);
			rv = S_ISFIFO(item);
			break;
		case 's':
//			printf("case %c\t", c);
			rv = S_ISSOCK(item);
			break;
		default:
			fprintf(stderr, "find -type: %c: unknown type.\n", c);
			break;
	}
	
	return rv;
}
