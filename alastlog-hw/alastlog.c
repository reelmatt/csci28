#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <lastlog.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include "lllib.h"

int check_time(time_t, long );
void fatal(char, char *);
void get_log(char *, char *, long);
void show_time(struct lastlog *, char *);
//void show_time(time_t, char *);
void process(char *user, int days, char *file);
void read_lastlog (struct lastlog *);
void show_llrec(struct lastlog *);
int show_info(struct lastlog *, struct passwd *, long, int);
void print_headers();
struct passwd *extract_user(char *);
void get_option(char, char **, char **, long *, char **);
long parse_time(char *);
char * check_string(char *, int);

#define LLOG_FILE "/var/log/lastlog"
#define TIME_FORMAT "%a %b %e %H:%M:%S %z %Y"
#define TIMESIZE 32
#define NO 0
#define YES 1

/*
 *	main()
 *		Process command-line arguments, if any, and then call get_log()
 *		to open the lastlog file (LLOG_FILE by default), with corresponding
 *      args, NULL if not specified.
 *
 *		Returns 0 on success, exits 1 and prints message to stderr on failure.
 */
int main (int ac, char *av[])
{
    int i = 1;
    char *user = NULL;
    long days = -1;
    char *file = NULL;
 
    /* Cycle through options, any/all of -u, -t, or -f. Exit if invalid.
     * get_option is only called when there is at least one more arg left
     * in addition to the '-' option, the (i + 1) < ac part in the if. */
    while (i < ac)
    {
    	if(av[i][0] == '-' && (i + 1) < ac)
            get_option(av[i][1], &av[i + 1], &user, &days, &file);
        else
    		fatal('\0', av[i]);

		i += 2;				//go past the -X option, and its value
	}

    //If no file specified with -f, use LLOG_FILE
    if (file == NULL)
        get_log(LLOG_FILE, user, days);
    else
        get_log(file, user, days);

	return 0;
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
 *	  Notes: If there is an invalid option (not -utf), fatal is called
 *			 to output a message to stderr and exit with a non-zero status.
 */
void get_option(char opt, char **value, char **user, long *days, char **file)
{
	if(opt == 'u')
		*user = *value;
	else if (opt == 't')
		*days = parse_time(*value);		//check if valid time, exit if not
	else if (opt == 'f')
		*file = *value;
	else
		fatal(opt, "");					//unrecognized option, exit with error

	return;
}

/*
 *	parse_time()
 *	Purpose: translate a DAY value into a corresponding time value
 *	  Input: value, the -t option the user entered
 *	 Return: the day value converted into a long
 *	 Errors: if the value is not a number, strtol will fail and the
 *			 program should exit
 */
long parse_time(char *value)
{
	char *temp = NULL;
	long time = strtol(value, &temp, 10);
	
	if(time == 0 && strcmp(value, temp) == 0)
	{
		fprintf(stderr, "alastlog: invalid numeric argument '%s'\n", value);
		exit(1);
	}
	
	return time;
}

/*
 *	extract_user()
 *	Purpose: obtain a passwd struct for a given username/UID
 *	  Input: name, the name/UID that was specified following -u
 *	 Return: a pointer to the passwd struct for the given name/UID.
 *	 Errors: If getpwnam() fails, the function tries to parse the
 *			 name into a UID. If it is determined to not be a number,
 *			 an invalid message is output to stderr. If successful,
 *			 but getpwuid() fails, then the "name" specified is
 *			 unknown, and we exit.
 */
struct passwd *extract_user(char *name)
{
	struct passwd *user = NULL;
	
	if ( (user = getpwnam(name)) != NULL)		//name was a username
		return user;
	else										//try name as a UID
	{
		char *temp = NULL;
        long uid = strtol(name, &temp, 10);
        
        //If strtol returns 0 and copied all chars to temp, it failed
        if (uid == 0 && strcmp(name, temp) == 0)
        {
        	fprintf(stderr, "alastlog: invalid user: %s\n", temp);
            exit (1);
        }
        
        //We were able to parse out a UID, try getting user with that
        if ( (user = getpwuid(uid)) == NULL)
        {
			fprintf(stderr, "alastlog: Unknown user: %s\n", name);
            exit(1);
        }
	}
	
	return user;
}

/*
 *	get_log()
 *	Purpose: Print out lastlog records, filtered as appropriate by user options
 *	  Input: file, lastlog to read from (LLOG_FILE by default)
 *			 user, specific username/UID to display record for
 *			 days, restrict output to logins within given number of days
 *	 Output: formatted headers and entries, through calling show_info
 *	 Errors: If there was a problem opening the lastlog file (ll_open) or
 *			 a problem extracting a provided user (extract_user), the program
 *			 will print a message to stderr and exit.
 */
void get_log(char *file, char *user, long days)
{
	if (ll_open(file) == -1)					//open lastlog file
	{
		perror(file);
		exit(1);
	}

	struct passwd *entry;						//store passwd rec
	struct lastlog *ll;							//store lastlog rec
	int headers = NO;							//have headers been printed
	
	if(user == NULL)
		entry = getpwent();						//open passwd db to iterate
	else
		entry = extract_user(user);				//get passwd rec for single user

	while (entry)								//still have a passwd entry
	{
		if ( ll_seek(entry->pw_uid) == -1 )		//get the correct pos in buffer
			ll = NULL;							//error
		else
			ll = ll_read();						//okay to read

        headers = show_info(ll, entry, days, headers);
          
        if( user != NULL)
        	return;								//found the user with -u, return
        else
        	entry = getpwent();					//go until end of passwd db
	}
	
	endpwent();									//if user specified, not called
	ll_close();									//close lastlog file
    return;
}

/*
 *	print_headers() - output formatted lastlog headers
 */
void print_headers()
{
	printf("%-16.16s ", "Username");
	printf("%-8.8s ", "Port");
	printf("%-16.16s ", "From");
    printf("%s", "Latest");
    printf("\n");

    return;
}

/*
 *
 */
int show_info(struct lastlog *lp, struct passwd *ep, long days, int headers)
{
    //if the lastlog is not NULL and time is OK, continue
	if (lp)
    {
        if (check_time(lp->ll_time, days) == NO)
            return headers;
    }
    else if (check_time(0, days) == NO)
    {
        return headers;
    }

    if (headers == NO)
        print_headers();

    printf("%-16.16s ", ep->pw_name);

    if (lp)
    {
        printf("%-8.8s ", check_string(lp->ll_line, UT_LINESIZE));
        printf("%-16.16s ", check_string(lp->ll_host, UT_HOSTSIZE));
    }
    else
    {
        printf("%-8.8s ", "");        
        printf("%-16.16s ", "");
    }

	if(lp == NULL || lp->ll_time == 0)
		printf("**Never logged in**");
	else
		show_time(lp, TIME_FORMAT);
		//show_time(lp->ll_time, TIME_FORMAT);

	printf("\n");

	return YES;
}

/*
 *	check_string()
 *	Purpose: see if the string is null terminated
 *	  Input: a string and the size of the string
 *	 Return: an empty string, if NULL; a null-terminated string
 *			 otherwise (could already be null-terminated when passed in).
 */
char * check_string(char *str, int size)
{
	if (str == NULL)
		return "";
	else if (str[size - 1] != '\0')
		str[size - 1] = '\0';
		
	return str;
}

/*
 *	show_time()
 *	Purpose: format a time and print it
 *	  Notes: copied, with slight modifications, from the who2.c code from
 *			 lecture #2.
 */
void show_time(struct lastlog *lp, char *fmt)
{
	if (lp == NULL || lp->ll_time == 0)
		printf("**Never logged in**");
			
	//struct tm *tp = localtime(&time);
	char result[TIMESIZE];
	strftime(result, TIMESIZE, fmt, lp->ll_time);

	printf("%s", result);
	return;
}

/*
 *	check_time()
 *	Purpose: see if the lastlogin time has occurred within the last days
 *	  Input: entry, the lastlogin time
 *			 days, specified by the user with the -t option
 *	 Return: NO, if login happened later than "days" ago
 *			 YES, if login has happened within the given # of "days"
 */
int check_time(time_t entry, long days)
{
	//check if a time was given with the -t flag
	if (days != -1)
	{
		time_t now;
		double delta = difftime(time(&now), entry);	//secs b/w now and lastlogin
        long day_seconds = 24 * 60 * 60;			//number of seconds in a day

		//login happened before DAYS ago, out of range
		if ( delta > (day_seconds * days) )
			return NO;
	}
	
	return YES;
}

/*
 *	fatal()
 *	Purpose: helper function to output error messages for bad input.
 */
void fatal(char opt, char *arg)
{
	if(opt == '\0')
		fprintf(stderr, "alastlog: unexpected argument: %s\n", arg);
	else
		fprintf(stderr, "alastlog: invalid option -- '%c'\n", opt);

	fprintf(stderr, "Usage: alastlog [options]\n\nOptions:\n");
	fprintf(stderr, "\t-u LOGIN\tprint lastlog record for user LOGIN\n");
	fprintf(stderr, "\t-t DAYS\t\tprint only records more recent than DAYS\n");
	fprintf(stderr, "\t-f FILE\t\tread data from specified FILE\n\n");

	exit(1);
}