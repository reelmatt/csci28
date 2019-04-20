/* builtin.c
 * contains the switch and the functions for builtin commands
 */

#include	<stdio.h>
#include	<string.h>
#include    <errno.h>
#include	<ctype.h>
#include	<stdlib.h>
#include    <unistd.h>
#include	<stdbool.h>
#include	"smsh.h"
#include	"varlib.h"
#include    "flexstr.h"
#include    "splitline.h"
#include	"builtin.h"

char * get_replacement(char * args, int *);
char var_or_comment(char * str, FLEXSTR s, char **prev);
void cntrl_err(char * msg);
void remove_comment(char * args);
int get_number(char * str);



int is_builtin(char **args, int *resultp)
/*
 * purpose: run a builtin command 
 * returns: 1 if args[0] is builtin, 0 if not
 * details: test args[0] against all known builtins.  Call functions
 */
{
	if ( is_assign_var(args[0], resultp) )
		return 1;
	if ( is_list_vars(args[0], resultp) )
		return 1;
	if ( is_export(args, resultp) )
		return 1;
	if ( is_cd(args, resultp) )
	    return 1;
	if ( is_exit(args, resultp) )
	    return 1;
    if ( is_read(args, resultp) )
	    return 1;
	return 0;
}

/* checks if a legal assignment cmd
 * if so, does it and retns 1
 * else return 0
 */
int is_assign_var(char *cmd, int *resultp)
{
	if ( strchr(cmd, '=') != NULL ){
		*resultp = assign(cmd);
		if ( *resultp != -1 )
			return 1;
	}
	return 0;
}

/* checks if command is "set" : if so list vars */
int is_list_vars(char *cmd, int *resultp)
{
	if ( strcmp(cmd,"set") == 0 ){	     /* 'set' command? */
		VLlist();
		*resultp = 0;
		return 1;
	}
	return 0;
}

/*
 * if an export command, then export it and ret 1
 * else ret 0
 * note: the opengroup says
 *  "When no arguments are given, the results are unspecified."
 */
int is_export(char **args, int *resultp)
{
	if ( strcmp(args[0], "export") == 0 ){
		if ( args[1] != NULL && okname(args[1]) )
			*resultp = VLexport(args[1]);
		else
			*resultp = 1;
		return 1;
	}
	return 0;
}

/*
 *  is_cd()
 *  Purpose: change directories
 *    Input: args, command line arguments
 *           resultp, where to store success of cd operation
 *   Return: 1 if built-in function, 0 otherwise. resultp is 0
 *           if chdir() was successful, 1 on error.
 */
int is_cd(char **args, int *resultp)
{
    if ( strcmp(args[0], "cd") == 0 )
    {
        if (args[1] == NULL && chdir(VLlookup("HOME")) == 0)
            *resultp = 0;     //go to HOME directory
        else if (args[1] != NULL && chdir(args[1]) == 0)
            *resultp = 0; //go to dir specified
        else
        {
            fprintf(stderr, "cd: %s: %s\n", args[1], strerror(errno));
            *resultp = 1;
        }
        
        return 1;       //was a built-in
    }
    
    return 0;
}


/*
 *	The Bourne shell: Cause the shell to exit with a status of n. If n is
 *	omitted, the exit status is that of the last command executed. A trap on
 *	EXIT is executed before the shell terminates.
 *
 *	dash shell states: Terminate the shell process. If exitstatus is given it
 *	is used as the exit status of the shell; otherwise the exit status of the
 *	preceding command is used.
 */
int is_exit(char **args, int *resultp)
{
	if( strcmp(args[0], "exit") == 0)
	{
		if( args[1] != NULL )               // exit arg specified?
		{
		    int val = get_number(args[1]);  // convert to a number

		    if (val != -1)                  // successful?
                exit(val);                  // use it
			else                            // syntax error
			{
			    cntrl_err(args[1]);
// 			    set_exit(2);
// 			    fatal("exit: Illegal number", args[1], 2);  // syntax error
// 			    fprintf(stderr, "exit: %s: %s\n", "Illegal number", args[1]);
// 			    exit(2);
				*resultp = 2;		//syntax error
			}
		}
		else
		{
            exit(get_exit());           //exit with last command's status		
		}
		
		return 1;           //was a built-in
	}
	
	return 0;
}

void cntrl_err(char * msg)
{
    fprintf(stderr, "exit: Illegal number: %s\n", msg);
//     set_exit(2);
    return;
}

/*
 *	get_number()
 *	Purpose: Helper function to check if str is a number
 *	  Input: str, the string to check
 *	 Return: -1 on error (str is not a number)
 *			 value returned from atoi() on success
 */
int get_number(char * str)
{
    int i;
    int len = strlen(str);
    
    for(i = 0; i < len; i++)
    {
        if (!isdigit(str[i]))
            return -1;
    }
    
	return atoi(str);
}

/*
 *
 */
int is_read(char **args, int *resultp)
{
//     char **varlist, **fieldlist;
    
    if ( strcmp(args[0], "read") == 0)
    {
    	if( okname(args[1]) )
    	{
    		char c;
			FLEXSTR s;
			fs_init(&s, 0);
    	
    		// read in value
    		while( (c = fgetc(stdin)) != EOF )
    		{
    			// end of command
    			if (c == '\n')
    				break;
    			
    			fs_addch(&s, c);	
    		}
    		
    		// null-terminate
    		fs_addch(&s, '\0');
    		fs_getstr(&s);
    		
			VLstore(args[1], fs_getstr(&s));
        }
        return 1;
    }
    
    return 0;
}


int assign(char *str)
/*
 * purpose: execute name=val AND ensure that name is legal
 * returns: -1 for illegal lval, or result of VLstore 
 * warning: modifies the string, but retores it to normal
 */
{
	char	*cp;
	int	rv ;

	cp = strchr(str,'=');
	*cp = '\0';
	rv = ( okname(str) ? VLstore(str,cp+1) : -1 );
	*cp = '=';
	return rv;
}

int okname(char *str)
/*
 * purpose: determines if a string is a legal variable name
 * returns: 0 for no, 1 for yes
 */
{
	char	*cp;

	for(cp = str; *cp; cp++ ){
		if ( (isdigit(*cp) && cp==str) || !(isalnum(*cp) || *cp=='_' ))
			return 0;
	}
	return ( cp != str );	/* no empty strings, either */
}

#define	is_delim(x) ((x)==' '|| (x)=='\t' || (x)=='\0')

char * varsub(char * args)
{
	char c, prev;
	FLEXSTR s;
	fs_init(&s, 0);
	int check;
	
	if (args == NULL)
	    return NULL;
	
	while ( (c = args[0]) )
	{
		if (c == '\\')                          // escape char
		{
			args++;
			fs_addch(&s, args[0]);
		}
		else if (c == '$')                      // variable sub
		{
			args++;                             //trim the $
			char *newstr = get_replacement(args, &check);
			printf("after sub, newstr is %s\n", newstr);
			args += (check - 1);

			fs_addstr(&s, newstr);
		}
		else if (c == '#' && is_delim(prev) )   // start of comment
		{
			break;                              // ignore the rest
		}
		else                                    // regular char
		{
			fs_addch(&s, c);                    // add as-is
		}
		
		prev = c;
		args++;
	}
	
	fs_addch(&s, '\0');
	char * return_str = fs_getstr(&s);
	fs_free(&s);
	return return_str;
}

char * special_replace(int val)
{
	FLEXSTR var;
	fs_init(&var, 0);
	
	char special_str[10] = "";
	sprintf(special_str, "%d", val);	//do error checking
	fs_addstr(&var, special_str);
	fs_addch(&var, '\0');
	return fs_getstr(&var);
// 	return (strcmp(special_str, "") == 0) ? fs_getstr(&var) : "";
	
//     return 1;
}

char * get_var(char *args, int * len)
{
	FLEXSTR var;
	fs_init(&var, 0);
	int skipped = 0;
	
	while (args[0])
	{
		fs_addch(&var, args[0]);
		skipped++;
			
		if( !(isalnum(args[0]) || args[0] == '_') )
			break;

		args++;

	}
	
	fs_addch(&var, '\0');
	
	char * str = fs_getstr(&var);
	fs_free(&var);
	
	*len = skipped;
	return str;
}

char * get_replacement(char * args, int * len)
{
	FLEXSTR sub;
	fs_init(&sub, 0);
// 	char special_str[10] = "";
// 	char * return_str;
	
// 	int skipped = 0;
	
	char * to_replace = get_var(args, len);

	printf("in get_replacement, str is %s\n", to_replace);

	if (strcmp(to_replace, "$") == 0)
		return special_replace(getpid());
	else if (strcmp(to_replace, "?") == 0)
		return special_replace(get_exit());
	else
		return VLlookup(to_replace);
}