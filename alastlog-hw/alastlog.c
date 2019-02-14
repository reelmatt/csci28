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
void show_time(time_t, char *);
void process(char *user, int days, char *file);
void read_lastlog (struct lastlog *);
void show_llrec(struct lastlog *);
int show_info(struct lastlog *, struct passwd *, long, int);
void print_headers();
struct passwd *extract_user(char *);
void get_option(char, char **, char **, long *, char **);
long parse_time(char *);

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
    //  printf("in get_option, address of days is %p\n", days);
	if(opt == 'u')
		*user = *value;
	else if (opt == 't')
		*days = parse_time(*value);
	//	*days = *value;
	else if (opt == 'f')
		*file = *value;
	else
		fatal(opt, "");

//    printf("parsed days is %lu\n", *days);
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
	
	if(name == NULL)							//return first user entry
		return getpwent();
	else if ( (user = getpwnam(name)) != NULL)	//name was a username
		return user;
	else										//try name as a UID
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

/*void logcmp(struct passwd *pw, struct lastlog *lp, long days, char *user, int p)
{
	if ( (int) pw->pw_uid < lp->ll_index )
		ll_reset(file);
	return;
}*/


/*
 *	get_log()
 *	Purpose: 
 *	  Input: file,
 *			 user,
 *			 days, 
 */
void get_log(char *file, char *user, long days)
{
	if (ll_open(file) == -1)
	{
		fprintf(stderr, "Could not open file, %s\n", file);
		perror(file);
		exit(1);
	}

	struct passwd *entry = extract_user(user);

	int headers = NO;
	int ll_index = -1;
	struct lastlog *ll;				//store lastlog rec
	
	//iterate through passwd structs and the lastlog file

	//while ( entry && (ll = ll_next()) )

//	while ( entry && ll_index < 65 )
	while (entry)
	{
		ll_index++;
		
        //set position in buffer, reloading as necessary
		if ( ll_seek(entry->pw_uid) == -1 )
			printf("THERE WAS AN ERROR WITH ll_seek()\n");

        //then read in the record once correct position is set
//		ll = ll_next();			
		ll = ll_read();
		
		
		
//		check_records(entry, ll, days, user, headers, ++ll_index);
		

		//if UID is before current lastlog record, need to reset
/*        if ( (int) entry->pw_uid < ll_index )
        {
//            printf("need to reset lastlog...\n");
            ll_reset(file);			
            ll_index = -1;
        }
*/		
		//@@TO-Do, remove this edge case
		//check if current entry in lastlog matches with /etc/passwd
		//if the lastlog does not match /etc/passwd, try next lastlog
/*		if (ll_index != (int) entry->pw_uid && entry->pw_uid < 65000)
		{
			continue;		//get next lastlog record
		}
*/
/*        if (check_time(ll->ll_time, days) == NO)
            continue;
*/

        
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

int check_time(time_t entry, long days)
{
	//check time against -t flag
	if (days != -1)
	{
		time_t now;
		double delta = difftime(time(&now), entry);
        long day_seconds = 24 * 60 * 60;

		//if out of range, don't print record
		if ( delta > (day_seconds * days) )
		{  
            //      printf("delta is %f\tuser is %lu\n", delta, (day_seconds * days));
			return NO;
		}
	}
	
	return YES;
}

/*
 *
 */
int show_info(struct lastlog *lp, struct passwd *ep, long days, int headers)
{
	if (check_time(lp->ll_time, days) == NO)
		return headers;
		
    if (headers == NO)
        print_headers();
        
	printf("%-16.16s ", ep->pw_name);
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
