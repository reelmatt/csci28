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
	
	print_headers();
	

	
	while( (ll = ll_next()) && (entry = getpwent()) )
	{
        if (user != NULL)
        {
            //printf("a specific user is specified, %s\n", user);
            
        }
         
        if (days != NULL)
        {
            time_t now;
            double delta = difftime(time(&now), ll->ll_time);
            
            if ( delta < (24 * 60 * 60 * atoi(days)) )
                show_info(ll, entry);
            //  else
            //   printf("not within %d days\n", atoi(days));

//            printf("difference is %f\n", test);
//            continue;
//            time_t now = time();
//            struct tim *mytime = ctime(&now);
            //printf("a specific time is specified, %d\n", atoi(days));
//            if ( ll->ll_time
//                show_info(ll, entry);
        }

		show_info(ll, entry);
	}
	
	endpwent();
	ll_close();

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
