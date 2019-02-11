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
        {
            get_option(av[i][1], &av[i + 1], &user, &days, &file);
    	}
        else
    	{
    		//fprintf(stderr, "alastlog: unexpected argument: %s\n", av[i]);
    		fatal('\0', av[i]);
    	}

		i += 2;
	}

    //If no file specified with -f, use LLOG_FILE
    if (file == NULL)
        get_log(LLOG_FILE, user, days);
    else
        get_log(file, user, days);

	return 0;
}

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

struct passwd *extract_user(char *name)
{
	struct passwd *user = NULL;
	
	if ( (user = getpwnam(name)) == NULL)
	{
//        printf("getpwnam() failed...\n");

        char *temp = NULL;
        long uid = strtol(name, &temp, 10);
        if (uid == 0 && strcmp(name, temp) == 0)
        {
            printf("invalid user ID conversion, remainder is %s\n", temp);
            exit (1);
            //return NULL;
        }


        if ( (user = getpwuid(uid)) == NULL)
        {

            //          printf("getpwuid() failed...\n");
            printf("alastlog: Unknown user: %s\n", name);
            exit(1);
//return NULL;
        }
	}
	
	return user;
}

void get_log(char *file, char *user, char *days)
{
	if (ll_open(file) == -1)
	{
        printf("Could not open file, %s\n", file);
		perror(file);
		exit(1);
	}

	struct lastlog *ll;				//store lastlog rec
	struct passwd *entry = NULL;	//store pw rec
	
	//get specific user, or first entry
	if (user == NULL)
    {
//        printf("user is NULL, access /etc/passwd\n");
        entry = getpwent();
    }
	else
    {	
        //      printf("user is specified, value is %s\n", user);
        entry = extract_user(user);
	}

    //tracking variables
	int headers = NO;
	int ll_index = -1;
	
	//cycle through valid passwd structs
	//either a single one with -u, or until end of /etc/passwd
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
        
/*		if (records == NO)
		{
			print_headers();
			records = YES;
		}
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

void print_headers()
{
	printf("%-16.16s ", "Username");
	printf("%-8.8s ", "Port");
	printf("%-16.16s ", "From");
    printf("%s", "Latest");
    printf("\n");

    return;
}

int show_info(struct lastlog *lp, struct passwd *ep, char *days, int headers)
{
	//check time against -t flag
//	printf("in show_info, days is %s\n", days);
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

void show_time(time_t time, char *fmt)
{
	struct tm *tp = localtime(&time);
	char result[100];

	strftime(result, 100, fmt, tp);

	printf("%s", result);
	return;
}

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
