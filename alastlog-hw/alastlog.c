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

int check_time(time_t, char *);
void fatal(char, char *);
void get_log(char *, char *, char *);
void show_time(time_t, char *);
void process(char *user, int days, char *file);
void process_option(char *, char*, char*, int, char*);
void read_lastlog (struct lastlog *);
void show_llrec(struct lastlog *);
int show_info(struct lastlog *, struct passwd *, char *, int);
void print_headers();
struct passwd *extract_user(char *);
void get_option(char, char **, char **, char **, char **);

#define LLOG_FILE "/var/log/lastlog"
#define TIME_FORMAT "%a %b %e %H:%M:%S %z %Y"
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
    char *days = NULL;
    char *file = NULL;
 
    /*Cycle through options, any/all of -u, -t, or -f. Exit if invalid*/
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
void get_option(char opt, char **value, char **user, char **days, char **file)
{
	if(opt == 'u')
		*user = *value;
	else if (opt == 't')
		*days = *value;
	else if (opt == 'f')
		*file = *value;
	else
		fatal(opt, "");

	return;
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
	
	//Try getting user by name first
	if ( (user = getpwnam(name)) == NULL)
	{
        char *temp = NULL;
        long uid = strtol(name, &temp, 10);
        
        //If strtol returns 0 and copied all chars to temp, it failed
        if (uid == 0 && strcmp(name, temp) == 0)
        {
        	fprintf(stderr, "alastlog: invalid username/UID: %s\n", temp);
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
 *	Purpose: 
 *	  Input: file,
 *			 user,
 *			 days, 
 */
void get_log(char *file, char *user, char *days)
{
	if (ll_open(file) == -1)
	{
		fprintf(stderr, "Could not open file, %s\n", file);
		perror(file);
		exit(1);
	}

	struct passwd *entry = NULL;	//store pw rec
	if (user == NULL)
        entry = getpwent();			//get first user in /etc/passwd
	else
        entry = extract_user(user);	//get specified user

	int headers = NO;
	int ll_index = -1;
	struct lastlog *ll;				//store lastlog rec
	
	//iterate through passwd structs and the lastlog file
	while ( entry && (ll = ll_next()) )
	{
		ll_index++;

		//if UID is before current lastlog record, need to reset
        if ( (int) entry->pw_uid < ll_index )
        {
//            printf("need to reset lastlog...\n");
            ll_reset(file);			
            ll_index = -1;
        }
		
		//@@TO-Do, remove this edge case
		//check if current entry in lastlog matches with /etc/passwd
		//if the lastlog does not match /etc/passwd, try next lastlog
		if (ll_index != (int) entry->pw_uid && entry->pw_uid < 65000)
		{
			continue;
		}
        
        headers = show_info(ll, entry, days, headers);
        
        //found the one user with -u, stop execution
        if( user != NULL)
        	return;
        else
        	entry = getpwent();
	}
	
	//if opened pwent, close
	if (user == NULL)
		endpwent();
	
	
	ll_close();
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

int check_time(time_t entry, char *days)
{
	//check time against -t flag
	if (days != NULL)
	{
		time_t now;
		double delta = difftime(time(&now), entry);

		//if out of range, don't print record
		if ( delta > (24 * 60 * 60 * atoi(days)) )
		{  
			return NO;
		}
	}
	
	return YES;
}

/*
 *
 */
int show_info(struct lastlog *lp, struct passwd *ep, char *days, int headers)
{
/*	//check time against -t flag
	if (days != NULL)
	{
		time_t now;
		double delta = difftime(time(&now), lp->ll_time);

		//if out of range, don't print record
		if ( delta > (24 * 60 * 60 * atoi(days)) )
		{  
			return headers;
		}
	}
*/
	if (check_time(lp->ll_time, days) == NO)
		return headers;
		
    if (headers == NO)
        print_headers();
        
	printf("%-16.16s ", ep->pw_name);
//    printf("%-8.8d ", ep->pw_uid);
	printf("%-8.8s ", lp->ll_line);        
	printf("%-16.16s ", lp->ll_host);

	if(lp->ll_time == 0)
		printf("**Never logged in**");
	else
		show_time(lp->ll_time, TIME_FORMAT);

	printf("\n");

	return YES;
}

/*
 *	show_time()
 *	Purpose: format a time and print it
 *	  Notes: copied, with slight modifications, from the who2.c code from
 *			 lecture #2.
 */
void show_time(time_t time, char *fmt)
{
	struct tm *tp = localtime(&time);
	char result[100];

	strftime(result, 100, fmt, tp);

	printf("%s", result);
	return;
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
