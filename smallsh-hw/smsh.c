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
static int execute_for();

static void setup();
static void io_setup();
static FILE * open_script(char *);
static int run_command(char * cmd);
static int run_shell = 1;

/*
 *	main()
 *	Purpose: 
 */
int main(int ac, char ** av)
{
	FILE * source;
	char *cmdline, *prompt;
	int result = 0;         //set default, if no cmds entered

	setup();    
    io_setup(&source, &prompt, ac, av);
    
    // get next line from 'source' and process accordingly
//	while ( (cmdline = next_cmd(prompt, source)) != NULL )
	while ( run_shell )
	{
	    cmdline = next_cmd(prompt, source);
// 	    printf("cmdline is...\n%s\n", cmdline);
	    if(cmdline == NULL)
	    {
// 	        printf("cmdline is NULL\n");
            run_shell = run_command(cmdline);
//             printf("returning, run_shell is '%d'\n", run_shell);
// 	        run_shell = process(cmdline);
// 	        run_shell = 0;
//             free(cmdline);
            
	        continue;
	    }
		
		// parsing for loop, direct input here
		if( is_parsing_for() )
		{
			// will be true when done loading, false while in process
			if (load_for_loop(cmdline) == true)
			{
				execute_for();
			}
            
//             printf("parsing for loop... continue\n");
			//
			continue;			
		}
		
		//all other commands/syntax
		result = run_command(cmdline);
	}
	return result;
}

/*
 *  execute_for()
 *  Purpose: 
 *   Return: 
 */
int execute_for()
{
	char **vars = get_for_vars();
	int result;
	
	// one loop for each var	
	while(*vars)
	{
		char * name = get_for_name();
	// 	printf("name = %s, val = %s\n", name, *vars);
		
		VLstore(name, *vars);
	
		char ** cmds = get_for_commands();
		
		// go through each command (for each var)
		while(*cmds)
		{
			result = run_command(*cmds);
			cmds++;
// 						free(cmdline);
		}
	
		vars++;
	}
	
	return 0;
}

/*
 *  run_command()
 *  Purpose: 
 *   Return: 
 */
int run_command(char * cmd)
{
// 	printf("about to varsub\n");
	char *subline = varsub2(cmd);
	char **arglist;
	int result = 0;

// 	printf("about to splitline\n");
	if ( (arglist = splitline(subline)) != NULL )
	{
// 	    printf("in the IF condition, splitline PASSED\n");
		result = process(arglist);
		last_exit = result;
// 		freelist(arglist);
	}
	else
	{
// 	    printf("in ELSE condition...\n");
// 	    arglist = NULL;
	    result = process(arglist);
	    clearerr(stdin);
// 	    free(cmd);
	}
	
// 	printf("returning from run_command\n");
// 	free(cmd);
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

/*
 *	get_exit() -- getter function to access $? value
 */
int get_exit()
{
    return last_exit;
}

/*
 *	io_setup
 *	Purpose: Detect if smsh should be run in interactive, or script mode.
 *			 Set 'FILE*' and 'prompt' accordingly.
 *	  Input: fp, address of FILE * back in main
 *			 pp, address of pointer to "prompt" back in main
 *			 args, number of command-line args
 *			 av, command-line args
 *	 Return: none
 *	 Errors: If a file is specified, but cannot be opened, open_script()
 *			 will output a message and exit.
 */
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

/*
 *  open_script()
 *  Purpose: Open a file, and handle any errors it encounters.
 *   Return: Pointer to the file (shell script) it opened
 */
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