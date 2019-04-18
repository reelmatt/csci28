/* controlflow.c
 *
 * "if" processing is done with two state variables
 *    if_state and if_result
 */
#include	<stdio.h>
#include 	<string.h>
#include	<stdbool.h>
#include	<stdlib.h>
#include	"smsh.h"
#include	"process.h"
#include	"flexstr.h"
#include	"varlib.h"

enum states   { NEUTRAL, WANT_THEN, THEN_BLOCK, ELSE_BLOCK, WANT_DO, WANT_DONE };
enum results  { SUCCESS, FAIL };



struct for_loop {
	char *varname;
	FLEXLIST varvalues;
	FLEXLIST commands;
};

struct for_loop * new_loop();

static struct for_loop * fl;
static int if_state  = NEUTRAL;
static int if_result = SUCCESS;
static int for_state = NEUTRAL;
static int last_stat = 0;

int	syn_err(char *);

int ok_to_execute()
/*
 * purpose: determine the shell should execute a command
 * returns: 1 for yes, 0 for no
 * details: if in THEN_BLOCK and if_result was SUCCESS then yes
 *          if in THEN_BLOCK and if_result was FAIL    then no
 *          if in WANT_THEN  then syntax error (sh is different)
 *   notes: Copied from starter-code for assignment 5. Code added to handle
 *          else blocks.
 */
{
	int	rv = 1;		/* default is positive */

	if ( if_state == WANT_THEN ){
		syn_err("then expected");
		rv = 0;
	}
	else if ( if_state == THEN_BLOCK && if_result == SUCCESS )
		rv = 1;
	else if ( if_state == THEN_BLOCK && if_result == FAIL )
		rv = 0;
	else if ( if_state == ELSE_BLOCK && if_result == SUCCESS )
		rv = 0;
	else if ( if_state == ELSE_BLOCK && if_result == FAIL )
		rv = 1;
	return rv;
}

int is_control_command(char *s)
/*
 * purpose: boolean to report if the command is a shell control command
 * returns: 0 or 1
 *   notes: Copied from starter-code for assignment 5. Code added to handle
 *          else blocks.
 */
{
    return (strcmp(s, "if") == 0 ||
            strcmp(s, "then") == 0 ||
            strcmp(s, "else") == 0 ||
            strcmp(s, "fi") == 0);
}

int is_for_loop(char *s)
{
    return (strcmp(s, "for") == 0 ||
            strcmp(s, "do") == 0 ||
            strcmp(s, "done") == 0);
}


int do_control_command(char **args)
/*
 * purpose: Process "if", "then", "fi" - change state or detect error
 * returns: 0 if ok, -1 for syntax error
 *   notes: I would have put returns all over the place, Barry says "no"
 *   notes: Copied from starter-code for assignment 5. Code added to handle
 *          else blocks.
 */
{
	char	*cmd = args[0];
	int	rv = -1;

	if( strcmp(cmd,"if")==0 ){
		if ( if_state != NEUTRAL )
			rv = syn_err("if unexpected");
		else {
			last_stat = process(args+1);
			if_result = (last_stat == 0 ? SUCCESS : FAIL );
			if_state = WANT_THEN;
			rv = 0;
		}
	}
	else if ( strcmp(cmd,"then")==0 ){
		if ( if_state != WANT_THEN )
			rv = syn_err("then unexpected");
		else {
			if_state = THEN_BLOCK;
			rv = 0;
		}
	}
	else if ( strcmp(cmd, "else") == 0) {
	    if( if_state != THEN_BLOCK )
	        rv = syn_err("else unexpected");
	    else {
	        if_state = ELSE_BLOCK;
	        rv = 0;
	    }
	}
	else if ( strcmp(cmd,"fi")==0 ){
		if ( if_state == THEN_BLOCK || if_state == ELSE_BLOCK) {
			if_state = NEUTRAL;
			rv = 0;
		}
		else {
			rv = syn_err("fi unexpected");
		}
	}
	else 
		fatal("internal error processing:", cmd, 2);
	return rv;
}

int init_for_loop(char **args)
{
	fl = new_loop();
	
	args++;	//strip the 'for'
	FLEXSTR name;
	FLEXLIST vars;
	
// 	printf("init for loop, first arg is: %s\n", *args);
	//if( valid_var(*args) )
	if ( true )
	{
		fs_init(&name, 0);

		while(*args[0] != '\0')
		{
// 			printf("string or char? %c\n", *args[0]);
			fs_addch(&name, *args[0]++);
		}
		
		fs_addch(&name, '\0');
		fl->varname = fs_getstr(&name);
		fs_free(&name);
		
// 		fl->varname = *args;
		args++;
		
		if(strcmp(*args, "in") == 0)
		{
// 			printf("correct placement of 'in'\n");
			args++;
			
			fl_init(&vars, 0);
			while(*args)
			{
				fl_append(&vars, *args);
				args++;
			}
			
			fl->varvalues = vars;
			
	// 		printf("parsed beginning of for\n");
// 			printf("varname is %s\nvarvalues are...", fl->varname);
// 			int i;
// 			char **test_list = fl_getlist(&fl->varvalues);
// 			for(i = 0; i < fl_getcount(&fl->varvalues); i++)
// 			{
// 				printf("%s\n", test_list[i]);
// 			}
			

			set_for(true);
			for_state = WANT_DO;
		}
		else
		{
			return syn_err("word unexpected (expecting \"in\")");
		}
		
	}
	
	return 0;
}

// returns TRUE when done loading for loop, FALSE otherwise
int load_for_loop(char *args)
{
	if(for_state == WANT_DO)
	{
		if(strcmp(args, "do") == 0)
		{
			
			for_state = WANT_DONE;
		}
		else
		{
			return syn_err("word unexpected (expecting \"do\")");

		}
	}
	else if (for_state == WANT_DONE)
	{
		if(strcmp(args, "done") == 0)
		{
// 			printf("in the done tract, freeing about to happen\n");
			
			
			printf("varname: %s\nvarvalues: \n", fl->varname);

			char **vals = fl_getlist(&fl->varvalues);
			char **cmds = fl_getlist(&fl->commands);
			
			while(*vals)
			{
				printf("%s\n", *vals++);
			}
			
			printf("\ncommands: \n");
			
			while(*cmds)
			{
				printf("%s\n", *cmds++);
			}
			
// 			if (fl)					//if for_loop struct was malloc'ed
// 				free(fl);			//free it for next time
			
			for_state = NEUTRAL;	//reset state
			return true;
		}
		
// 		FLEXSTR cmd;
// 		fs_init(&cmd, 0);
// 		
// 		while(*args && (for_loop.num_cmds < MAXCMDS) )
// 		{
// 			fl_append(&cmd, *args);
// 			args++;
// 		
// 		}

		fl_append(&fl->commands, args);

	}
	else
	{
		printf("load_for_loop syntax error. arg is %s\n", args);
	}
	
	return false;
}

char * get_next_cmd()
{
	char **vars = fl_getlist(&fl->varvalues);
	
	while(*vars)
	{
// 		printf("name = %s, val = %s\n", fl->varname, *vars);
		VLstore(fl->varname, *vars);
		
		char **commands = fl_getlist(&fl->commands);
		
		while(*commands)
		{
			return *commands;
		}
	}
	
	return NULL;
}

char ** get_for_commands()
{
	return fl_getlist(&fl->commands);
}

char ** get_for_vars()
{
	return fl_getlist(&fl->varvalues);
}

char * get_for_name()
{
	return fl->varname;
}

int is_parsing_for()
{
	return for_state != NEUTRAL;
}

int syn_err(char *msg)
/* purpose: handles syntax errors in control structures
 * details: resets state to NEUTRAL
 * returns: -1 in interactive mode. Should call fatal in scripts
 */
{
	if_state = NEUTRAL;
	fprintf(stderr,"syntax error: %s\n", msg);
	return -1;
}


struct for_loop * new_loop()
{
	struct for_loop * lp = malloc(sizeof(struct for_loop));
	
	if(lp == NULL)
	{
		fprintf(stderr, "./smsh: Couldn't allocate memory for a for loop.\n");
		exit(5);
	}
	
	return lp;
}