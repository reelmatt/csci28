#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>
// #include    <fcntl.h>
#include	"smsh.h"
#include	"splitline.h"
#include	"varlib.h"
#include	"process.h"
#include    "builtin.h"

/**
 **	small-shell version 5
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **
 **     hist: 2017-04-12: changed comment to say "version 5"
 **/

#define	DFL_PROMPT	"> "

void	setup();

int main(int ac, char ** av)
{
	char	*cmdline, *subline, *prompt, **arglist;
	int	result;
	int script_fd;
// 	int script_fd = -1;
    FILE * script = NULL;

	prompt = DFL_PROMPT ;
	setup();

    if(ac >= 2)
    {
//        printf("this is a shell script...\n");
  //      printf("file name is: %s\n", av[1]);
        
        script = fopen(av[1], "r");
        
        if (script == NULL)
        {
            fprintf(stderr, "Can't open %s\n", av[1]);
            exit(127);
        }
        
        prompt = "";
//         script_fd = open(av[1], O_RDONLY);
    }
    FILE * to_run = (script) ? script : stdin;
    
	while ( (cmdline = next_cmd(prompt, to_run)) != NULL ){
	    subline = varsub(cmdline);
// 	    printf("Subline is: %s\n", subline);
	    
		//if ( (arglist = splitline(cmdline)) != NULL  ){
		if ( (arglist = splitline(subline)) != NULL  ){
            result = process(arglist);
			freelist(arglist);
		}
		free(cmdline);
	}
	return result;
}

void setup()
/*
 * purpose: initialize shell
 * returns: nothing. calls fatal() if trouble
 */
{
	extern char **environ;

	VLenviron2table(environ);
	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

void fatal(char *s1, char *s2, int n)
{
	fprintf(stderr,"Error: %s,%s\n", s1, s2);
	exit(n);
}
