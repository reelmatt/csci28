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
#include    "flexstr.h"

/**
 **	small-shell version 5
 **		first really useful version after prompting shell
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **
 **     hist: 2017-04-12: changed comment to say "version 5"
 **/

#define	DFL_PROMPT	"> "


static int last_exit = 0;
static int execute_for();
static int shell_mode = INTERACTIVE;

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
	int result = 0;                             // set default

	setup();    
    io_setup(&source, &prompt, ac, av);
    
	while ( run_shell )
	{
	    cmdline = next_cmd(prompt, source);     // get next line from source
	    
	    if(cmdline == NULL)                     // cmdline was EOF
	    {
	    	run_shell = safe_to_exit();			// check if processing if/for
	    	clearerr(stdin);					// clear the EOF
//             run_shell = run_command(cmdline);   // check is_safe_to_exit()
	        continue;
	    }
		
		if( is_parsing_for() )                  // reading in a for_loop
		{
			if (load_for_loop(cmdline) == true) // when true
				result = execute_for();         // for_loop complete, execute

			continue;			                // go to next cmdline
		}
		
		result = run_command(cmdline);          // all other commands/syntax
	}
	return result;
}

/*
 *  execute_for()
 *  Purpose: Iterate through a completed for_loop struct and execute cmds
 *   Return: 
 */
int execute_for()
{
	int result;
	char **vars = get_for_vars();           // load in varvalues
    char * name = get_for_name();           // load in varname for sub
	char ** vars_start = vars;              // keep track of memory
	char ** cmds_start;

	while(*vars)                            // for each varvalue
	{
		VLstore(name, *vars);               // set current var for varsub
		char ** cmds = get_for_commands();  // get array of commands
		cmds_start = cmds;          // keep track of memory
		
		while(*cmds)                        // go through cmds for each var
		{
			result = run_command(*cmds);    // execute
			cmds++;
		}
		

		vars++;                             // next variable
	}
	
// 	if(cmds_start)                      // if malloc'ed
// 		fl_freelist(cmds_start);        // free it
	if(vars_start)
        fl_freelist(vars_start);                 
    if(name)
        free(name);
    free_for();
	
	printf("AFTER free_for()\n");
	return 0;
}

/*
 *  run_command()
 *  Purpose: 
 *   Return: 
 */
int run_command(char * cmdline)
{
	char *subline = varsub(cmdline);
	char **arglist;
	int result = 0;

	if ( (arglist = splitline(subline)) != NULL )
	{

		result = process(arglist);
// 		freelist(arglist);
	}
// 	else
// 	{
// // 	    arglist = NULL;
// 	    result = process(arglist);
// 	    clearerr(stdin);
// // 	    free(cmd);
// 	}
	
	//syntax error 
	if(result == -1)
		result = 2;
	
// 	if(cmdline)
// 		free(cmdline);
	
	freelist(arglist);
	
	if(subline)
		free(subline);

	set_exit(result);
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

void set_exit(int status)
{
    last_exit = status;
    return;
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
		shell_mode = SCRIPTED;
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

int get_mode()
{
	return shell_mode;
}