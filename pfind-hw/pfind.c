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
	int rv = 0;

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

	printf("after processing, vars are %s, %s, %c\n", path, name, type);
	
	if (path)
		searchdir(path, name, type);
// 	else
// 		searchdir(".", name, type);

	return rv;
}

void get_path(char *val, char **path)
{
	if(val[0] != '-')
		*path = val;
	else
	{
		fprintf(stderr, "not a valid path\n");
		exit (1);
	}
	
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

void searchdir(char *dirname, char *findme, char type)
{
	//printf("passed in vars are %s, %s, %c\n", dirname, findme, type);
	
	DIR* current_dir;
	struct dirent *dp = NULL;
	struct stat *info = new_stat();
	
	//open the directory, exit on error
	if ( (current_dir = opendir(dirname)) == NULL)
	{
		perror(dirname);
		exit(1);
	}
	
	//iterate through all entries in the director
	while( (dp = readdir(current_dir)) != NULL )
	{
		char *full_path = construct_path(dirname, dp->d_name);

		//printf("in while loop, what is full_path? - %s\n", full_path);
		
		if (lstat(full_path, info) == -1)
		{
			perror(full_path);
			continue;
		}

		//if either/both options specified, check against filter
		if (findme || type != '\0')
		{
			if (fnmatch(findme, dp->d_name, FNM_PERIOD) == 0)
				printf("findme is %s and full_path is %s\n", findme, full_path);
			else
				printf("does not match\n");
			//printf("type is %c and check is %d\n", type, check_type(type, info->st_mode));
		}
		else
		{
			printf("%s\t\tno options specified...\n", full_path);
		}

		if (S_ISDIR(info->st_mode))
		{
			if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
			{
				//printf("current, or above dir. don't process\n");
				continue;
			}
			
			//printf("recurse now! full_path is %s\n", full_path);
//			printf("%s\t%hu\n", full_path, info->st_mode);
			searchdir(full_path, findme, type);
		}
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
			rv = S_ISBLK(item);
			break;
		case 'c':
			rv = S_ISCHR(item);
			break;
		case 'd':
			rv = S_ISDIR(item);
			break;
		case 'f':
			rv = S_ISREG(item);
			break;
		case 'l':
			rv = S_ISLNK(item);
			break;
		case 'p':
			rv = S_ISFIFO(item);
			break;
		case 's':
			rv = S_ISSOCK(item);
			break;
		default:
			fprintf(stderr, "find -type: %c: unknown type.\n", c);
			break;
	}
	
	return 0;
}
