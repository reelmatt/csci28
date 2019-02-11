#include <stdio.h>
#include <lastlog.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include "lllib.h"

void get_log(char *, char *, char *);
void show_time(time_t, char *);
void process(char *user, int days, char *file);
void process_option(char *, char*, char*, int, char*);
void read_lastlog (struct lastlog *);
void show_llrec(struct lastlog *);
void show_info(struct lastlog *, struct passwd *);
void print_headers();

#define LLOG_FILE "/var/log/lastlog"
#define TIME_FORMAT "%a %b %d %H:%M:%S %z %Y"
#define NO 0
#define YES 1

int main (int ac, char *av[])
{
    int i = 1;
    char *user = NULL;
    char *days = NULL;
    char *file = NULL;
    
    /*Cycle through command-line args to store any options user specifies.
      Known options are -u, -t, and -f. Each option requires an arg following
      it.*/
    while (i < ac)
    {
		if(av[i][0] == '-' && (i + 1) < ac && av[i+1][0] != '-')
		{
			if(av[i][1] == 'u')
                user = av[i+1];
			else if(av[i][1] == 't')
				days = av[i+1];
			else if (av[i][1] == 'f')
                file = av[i+1];
            else
                printf("Unknown option. Please try again.\n");
		}
		else
			printf("usage: ./alastlog [-utf] [file]\n");

		i += 2;
	}

    //If no file specified with -f, use LLOG_FILE
    if (file == NULL)
        get_log(LLOG_FILE, user, days);
    else
        get_log(file, user, days);

	return 0;
}

void get_log(char *file, char *user, char *days)
{
//	printf("user is %s and file is %s\n", user, file);
	int headers = NO;
	struct lastlog *ll;
	
	
	if (ll_open(file) == -1)
	{
        printf("Could not open file, %s\n", file);
		perror(file);
		return;
	}
	
	struct passwd *entry = NULL;
	
	//if a -u user is specified, extract single passwd struct
	if (user != NULL)
	{
		if ( (entry = getpwnam(user)) == NULL )
		{
			printf("alastlog: Unknown user: %s\n", user);
			return;
		}
	}
	else
	{
		entry = getpwent();
	}
	
	int ll_index = -1;
	
	//cycle through valid passwd structs
	//either a single one with -u, or until end of /etc/passwd
	while ( entry && (ll = ll_next()) )
	{
		ll_index++;

        if (ll_index > (int) entry->pw_uid)
            printf("reset lastlog position\n");
		
		//check if current entry in lastlog matches with /etc/passwd
		if (ll_index != (int) entry->pw_uid && entry->pw_uid < 65000)
		{
            printf("ll_index does not match UID, index is %d\tUID is %d\n", ll_index, entry->pw_uid);
			continue;
		}
		
		//check time against -t flag
		if (days != NULL)
        {
            time_t now;
            double delta = difftime(time(&now), ll->ll_time);
            
            if ( delta > (24 * 60 * 60 * atoi(days)) )
                continue;
        }
        
        if (headers == NO)
        {
            print_headers();
            headers = YES;
        }
        
        show_info(ll, entry);
        
        //found the one user with -u, stop execution
        if( user != NULL)
        	return;
        else
        	entry = getpwent();
	}
	
	
/*
	//iterate through the lastlog database
	while( (ll = ll_next()) )
	{
	
		

		show_info(ll, entry);
        entry = getpwent();
        current_uid++;
        
		printf("%-16.16s ", "");
		printf("%-8.8s ", ll->ll_line); 
		printf("%-16.16s ", ll->ll_host);
		printf("%d", ll->ll_time);
//		show_time(ll->ll_time, TIME_FORMAT);
        printf("\n");
	}
*/
//	struct passwd *single;
//    single = getpwnam("mst611");
//    printf("retrieved user... name is %s, uid is %d, home is %s\n", single->pw_name, single->pw_uid, single->pw_dir);
//	struct passwd *entry = getpwent();

/*
	while( (ll = ll_next()) && entry )
	{
        if (user != NULL)
        {
            if (strcmp(entry->pw_name, user) != 0)
                continue;            
        }
         
        if (days != NULL)
        {
            time_t now;
            double delta = difftime(time(&now), ll->ll_time);
            
            if ( delta > (24 * 60 * 60 * atoi(days)) )
                continue;
        }

        if (headers == NO)
        {
            print_headers();
            headers = YES;
        }

		show_info(ll, entry);
        entry = getpwent();
	}*/
	
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

void show_info(struct lastlog *lp, struct passwd *ep)
{
	printf("%-16.16s ", ep->pw_name);
    printf("%-8.8d ", ep->pw_uid);
	printf("%-8.8s ", lp->ll_line);        
	printf("%-16.16s ", lp->ll_host);

	if(lp->ll_time == 0)
		printf("**Never logged in**");
	else
		show_time(lp->ll_time, TIME_FORMAT);

	printf("\n");

	return;
}

void show_time(time_t time, char *fmt)
{
	struct tm *tp = localtime(&time);
	char result[100];

	strftime(result, 100, fmt, tp);

	printf("%s", result);
	return;
}
