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

void show_time(time_t, char *);
void process(char *user, int days, char *file);
void process_option(char *, char*, char*, int, char*);
void read_lastlog (struct lastlog *);
void show_llrec(struct lastlog *);
void show_info(struct lastlog *, struct passwd *);

#define LLOG_FILE "/var/log/lastlog"

int main (int ac, char *av[])
{
	int i = 1;
	char user[UT_LINESIZE];
	int days;
	char file[255];

	while (i < ac)
	{
		if(av[i][0] == '-')
		{
			if(av[i][1] == 'u')
				strcpy(user, av[i+1]);
//			else if(av[i][1] == 't')
//				days = av[i+1];
			else if (av[i][1] == 'f')
				strcpy(file, av[i+1]);

		}
		else
			printf("usage: ./alastlog [-utf] [file]\n");

//			process_option(av[i], av[i+1], &user, &days, &file);

		i += 2;
	}

//	printf("user is %s, days is %d, and file is %s\n", user, days, file);

	struct lastlog llbuf;
	int llfd;

	if( (llfd = open(LLOG_FILE, O_RDONLY)) == -1 )
	{
		fprintf(stderr, "%s: cannout open %s\n", *av, "/var/log/lastlog");
		exit(1);
	}
	struct passwd *entry;
//	printf("should be open, fd was %d\n", llfd);
	printf("%-16s %-8s %-16s %-1s\n", "Username", "Part", "From", "Latest");
	while( read(llfd, &llbuf, sizeof(llbuf)) == sizeof(llbuf) )
	{
//		printf("read data\n");
		entry = getpwent();
		show_info(&llbuf, &entry);
	}
	close(llfd);
	return 0;

//	process(user, days, file);

//	struct lastlog ll = { 0 };
//	read_lastlog(&ll);
//	fprintf(stdout, "%d %10s %10s\n", ll.ll_time, ll.ll_line, ll.ll_host);
//	return 0;
}

void show_info(struct lastlog *lp, struct passwd *ep)
{
//	printf("lp ut_type is %d\n", lp->ut_type);
//	printf("in show_info...\n");
//	printf("%-16s %-8s %-16s %-1d\n", lp->ll_line, lp->ll_host, lp->ll_time);
	printf("%-16.16s ", ep->pw_name);
	printf("%-8.8s ", lp->ll_line);
	printf("%-16.16s", lp->ll_host);
//	printf("%-16.16s %-8.8s %-16.16s ", "", lp->ll_line, lp->ll_host);

	if(lp->ll_time == 0)
		printf("**Never logged in**");
	else
		show_time(lp->ll_time, "%a %b %d %H:%M:%S %z %Y");
	//	printf("%-1d\n", lp->ll_time);

	printf("\n");
//	fprintf(stdout, "%d %10s %10s\n", lp->ll_time, lp->ll_line, lp->ll_host);
	return;
}

void show_time(time_t value, char *fmt)
{
	char result[100];

//	char *time = ctime(value);

	struct tm *tp = localtime(&value);
	strftime(result, 100, fmt, tp);
//	fputs(result, stdout);
	printf("%s", result);
	return;
}

void process(char *user, int days, char *file)
{
	struct lastlog *ll;
	struct lastlog *ll_next();

	if (ll_open(file) == -1)
	{
		perror(file);
		return;
	}

	while ( (ll = ll_next()) != NULL )
		show_llrec(ll);

	ll_close();
	return;
}

void show_llrec(struct lastlog *lp)
{
	printf("%d %10s %10s\n", lp->ll_time, lp->ll_line, lp->ll_host);
	putchar('\n');
	return;
}

void process_option(char *option, char *value, char *user, int days, char *file)
{
	printf("In process_option...\n");
	printf("option is %c\n", option[1]);
	printf("value is %s\n", value);

	if (option[1] == 'u')
		*user = value;
	else if (option[1] == 't')
		days = 5;
	else if (option[1] == 'f')
		*file = value;
	else
		printf("Unknown option, try again\n");

	return;
}

void read_lastlog (struct lastlog *ll)
{
        fread(ll, sizeof(struct lastlog), 1, stdin);
}
