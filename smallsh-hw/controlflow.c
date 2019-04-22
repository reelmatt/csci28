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
#include	"controlflow.h"
#include	"splitline.h"
#include	"process.h"
#include    "builtin.h"
#include	"flexstr.h"
#include	"varlib.h"

enum states   { NEUTRAL, WANT_THEN, THEN_BLOCK, ELSE_BLOCK, WANT_DO, WANT_DONE };
enum results  { SUCCESS, FAIL };

struct for_loop {
	FLEXSTR varname;
	FLEXLIST varvalues;
	FLEXLIST commands;
};

static struct for_loop fl;
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

int safe_to_exit()
{
//     printf("in safe_to_exit()\n");
    if (if_state != NEUTRAL || for_state != NEUTRAL)
    {
        syn_err("end of file unexpected (expecting...)");
        //set_exit(2);
        return 1;
    }
    
    return 0;
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

/*
 *	is_for_loop()
 *	Purpose: boolean to report if the command is a for loop command
 *	 Return: 0 or 1
 *	   Note: This function mimics the is_control_command() included in the
 *			 starter code; just with the for loop keywords.
 */
int is_for_loop(char *s)
{
    return (strcmp(s, "for") == 0 ||
            strcmp(s, "do") == 0 ||
            strcmp(s, "done") == 0);
}

int do_for_loop(char **args)
{
	char *cmd = args[0];
	int rv = -1;
	
	if (strcmp(cmd, "for") == 0)
	{
		if (for_state != NEUTRAL)
			rv = syn_err("for unexpected");
		else {
			rv = init_for_loop(args);
			for_state = WANT_DO;
		}
	}
	else if (strcmp(cmd, "do") == 0)
	{
		if (for_state != WANT_DO)
			rv = syn_err("do unexpected");
	}
	else if (strcmp(cmd, "done") == 0)
	{
		if (for_state != WANT_DONE)
			rv = syn_err("done unexpected");
	}
	else
		fatal("internal error processing:", cmd, 2);
		
	return rv;
}

int do_control_command(char **args)
/*
 * purpose: Process "if", "then", "else", "fi" - change state or detect error
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

void load_for_varname(char * str)
{
    FLEXSTR name;
    fs_init(&name, 0);
    fs_addstr(&name, str);
    fs_addch(&name, '\0');
    fl.varname = name;
}

void load_for_varvalues(char **args)
{
    FLEXLIST vars;
    fl_init(&vars, 0);
    while(*args)
    {
        fl_append(&vars, *args);
        args++;
    }
    
    fl.varvalues = vars;
}

int init_for_loop(char **args)
{	
	args++;	//strip the 'for'
	
	if ( okname(*args) )                // valid varname
	{
	    load_for_varname(*args++);      // store in struct, strip from args
		
		if(strcmp(*args, "in") == 0)    // validate "in"
		{
			load_for_varvalues(++args); // load any args after that
			for_state = WANT_DO;        // change state
		}
		else
		{
			return syn_err("word unexpected (expecting \"in\")");
		}
	}
	else
	{
	    syn_err("Bad for loop variable");
	}
	
	return 0;
}

// returns TRUE when done loading for loop, FALSE otherwise
int load_for_loop(char *args)
{	
	char **arglist = splitline(args);
	
	if(arglist == NULL || arglist[0] == NULL)   // check we have args
	    return false;                           // we don't

// 	char *cmd = arglists[0];
	
	if(for_state == WANT_DO)
	{
		if(strcmp(arglist[0], "do") == 0)
			for_state = WANT_DONE;
		else
			return syn_err("word unexpected (expecting \"do\")");
	}
	else if (for_state == WANT_DONE)
	{
		if(strcmp(arglist[0], "done") == 0) // reached the end?
		{
			for_state = NEUTRAL;	        // reset state
			return true;                    // done loading
		}

		fl_append(&fl.commands, args);      // not a 'done', load raw command
	}
	else
		fatal("internal error processing:", arglist[0], 2);
	
	
	//BIG glibc problem
// 	freelist(arglist);
// 	free(args);
	
	return false;
}

char ** get_for_commands()
{
	return fl_getlist(&fl.commands);
}

char ** get_for_vars()
{
	return fl_getlist(&fl.varvalues);
}

char * get_for_name()
{
	return fs_getstr(&fl.varname);
}

void free_for()
{
// 	printf("got to free_for()\n");
//     if(fl.varname)              // check if NULL
        fs_free(&fl.varname);   // free it

    fl_free(&fl.varvalues);     //flexstr checks if NULL
    fl_free(&fl.commands);      //flexstr checks if NULL
    
    return;
}

/*
 *  is_parsing_for()
 *  Purpose: Check if shell is currently reading in a for_loop struct
 *   Return: 1 if reading in a for_loop, 0 if not
 */
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
	if(get_mode() == SCRIPTED)
		fatal("syntax error: ", msg, 2);
	
	
	if_state = NEUTRAL;
	for_state = NEUTRAL;
	fprintf(stderr,"syntax error: %s\n", msg);
	set_exit(2);
	
// 	else
		
	
	return -1;
}