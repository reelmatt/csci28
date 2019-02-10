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

	struct lastlog *ll;
	struct lastlog *ll_next();
	struct passwd *entry;
	
	if (ll_open(file) == -1)
	{
        printf("Could not open file\n");
		perror(file);
		return;
	}
	
//	print_headers();
    int headers = NO;

//	struct passwd *single;

//    single = getpwnam("mst611");
//    printf("retrieved user... name is %s, uid is %d, home is %s\n", single->pw_name, single->pw_uid, single->pw_dir);

//    return;
    entry = getpwent();

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
	}
	
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
//    printf("%-16.16s ", "");
//    printf("%s\t%s\t", lp->ll_line, lp->ll_host);
/*
    if(lp->ll_line[UT_LINESIZE] == '\0')
*/      printf("%-8.8s ", lp->ll_line);        
    /*   else
    {
        char temp[UT_LINESIZE];
        strcpy(temp, lp->ll_line);
        temp[UT_LINESIZE - 1] = '\0';

        if(temp[UT_LINESIZE] == '\0')
            printf("%-8.8s ", temp);
        else
            printf("%-s %s", "not string ", temp);
    }

    if(lp->ll_host[UT_HOSTSIZE] == '\0')
    */   printf("%-16.16s ", lp->ll_host);
    /*   else
    {
        char temp2[UT_HOSTSIZE];
        strcpy(temp2, lp->ll_host);
        temp2[UT_HOSTSIZE - 1] = '\0';
        
        if(temp2[UT_HOSTSIZE] == '\0')
            printf("%-16.16s ", temp2);
        else
            printf("%-16.16s ", "not host");

    }
//        printf("%-16.16s ", "not host string");
*/
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
