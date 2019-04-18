#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>
#include	<stdbool.h>
// #include    <fcntl.h>
#include	"smsh.h"
#include	"splitline.h"
#include	"controlflow.h"
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


static int last_exit = 12;
static int parsing_for = false;

static void setup();
static void io_setup();
static FILE * open_script(char *);
static int run_command(char * cmd);

void io_setup(FILE ** fp, char ** pp, int args, char ** av)
{
	if(args >= 2)
	{
		*fp = open_script(av[1]);
		*pp = "";
	}
	else
	{
		*fp = stdin;
		*pp = DFL_PROMPT;
	}
	
	return;
}

FILE * open_script(char * file)
{
	FILE * fp = fopen(file, "r");
	
	if(fp == NULL)
	{
		fprintf(stderr, "Can't open %s\n", file);
		exit(127);
	}
	
	return fp;
}

int main(int ac, char ** av)
{
	FILE * source;
	char *cmdline, *prompt;
	int result;

	setup();    
    io_setup(&source, &prompt, ac, av);
    
    // get next line from 'source' and process accordingly
	while ( (cmdline = next_cmd(prompt, source)) != NULL )
	{
		// parsing for loop, direct input here
		if( is_parsing_for() )
		{
// 			printf("parsing for...\n");
// 			if (load_for_loop(cmdline) == true)
// 			{
// 				
// 			}
			
			
			/*if (is_done())		//&& ok_to_execute()?
			{
				
			}
			
			//done loading
			if (rv == true) //&& ok_to_execute()?
			{
				//loop over for loop struct
				char * cmd;
				while( (cmd = get_next_cmd()) != NULL)
				{
					result = run_command(cmd);				
				}
			}*/
			// will be true when done loading, false while in process
			if (load_for_loop(cmdline) == true)
			{
				char * to_run;
				
// 				char *name = get_for_name();
// 				char **vals = get_for_vals();
// 				char **cmds = get_for_cmds();
// 				
// 				while (*vals)
// 				{
// 				
// 				}
				
		// 		while another command in for loop
// 				while( (to_run = get_next_cmd()) != NULL)
// 				{
// 					result = run_command(to_run);
// 				}
// 				printf("Done loading the for loop. Now on to processing\n");
// 				char ** for_vars = get_for_vars();
// 
				char **for_vars = get_for_vars();
				
				while(*for_vars)
				{
					char * name = get_for_name();
// 					printf("name = %s, val = %s\n", name, *for_vars);
					
					VLstore(name, *for_vars);
				
					char ** for_commands = get_for_commands();
					while(*for_commands)
					{
						result = run_command(*for_commands);
						for_commands++;
// 						free(cmdline);
					}
				
					for_vars++;
				}
				parsing_for = false;
			}

			//
			continue;			
		}
		
		//all other commands/syntax
		result = run_command(cmdline);
	}
	return result;
}

int run_command(char * cmd)
{
	char *subline = varsub(cmd);
	char **arglist;
	int result = 0;
	
	if ( (arglist = splitline(subline)) != NULL )
	{
		result = process(arglist);
		last_exit = result;
// 		freelist(arglist);
	}
	
// 	free(cmd);
	return result;	
}

int do_for(char * cmdline)
{
	
	return false;
}

int loop_thru_for()
{
	
	return 0;
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

void set_for(int state)
{
	parsing_for = state;
	return;
}

int get_for()
{
	return parsing_for;
}

int get_exit()
{
    return last_exit;
}
