/* builtin.c
 * contains the switch and the functions for builtin commands
 */

#include	<stdio.h>
#include	<string.h>
#include    <errno.h>
#include	<ctype.h>
#include	<stdlib.h>
#include    <unistd.h>
#include	"smsh.h"
#include	"varlib.h"
#include    "flexstr.h"
#include    "splitline.h"
#include	"builtin.h"

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
 *
 */
int is_cd(char **args, int *resultp)
{
    if ( strcmp(args[0], "cd") == 0 )
    {
//         int ch_result;

// to use instead of getenv("HOME") @@TO-DO        
// https://www.stev.org/post/cgethomedirlocationinlinux
// https://stackoverflow.com/questions/2910377/get-home-directory-in-linux

        if (args[1] == NULL && chdir(getenv("HOME")) == 0)
            *resultp = 0;     //go to HOME directory
        else if (args[1] != NULL && chdir(args[1]) == 0)
            *resultp = 0; //go to dir specified
        else
        {
            fprintf(stderr, "cd: %s: %s\n", args[1], strerror(errno));
//             perror("cd: ");
//             fprintf(stderr, "cd: %s: No such file or directory", args[1]);
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
		if( args[1] != NULL )
		{
			if ( isdigit((int) args[1]) )
			    exit((int) args[1]);
// 				*resultp = (int) args[1];
			else
			{
			    fprintf(stderr, "exit: %s: %s\n", "Illegal number", args[1]);
			    exit(2);
// 				*resultp = 2;		//syntax error
			}
		}
		exit(0);				//exit with last command's status
		return 1;           //was a built-in
	}
	return 0;
}

/*
 *
 */
int is_read(char **args, int *resultp)
{
    char **varlist, **fieldlist;
    
    if ( strcmp(args[0], "read") == 0)
    {
//         printf("*args is %s\n", *args);
        args++;
        printf("args is now %s\n", *args);
        varlist = args;
        printf("varlist: %s\n", varlist[0]);
        if(varlist != NULL)
        {
            VLstore(varlist[0], "temp");
            
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
/*
 * step through args.  REPLACE any arg with leading $ with the
 * value for that variable or "" if no match.
 * note: this is NOT how regular sh works
 */
// void varsub(char **args)
// {
// 	int	i;
// 	char	*newstr;
// 
// 	for( i = 0 ; args[i] != NULL ; i++ )
// 		if ( args[i][0] == '$' ){
// 			newstr = VLlookup( args[i]+1 );
// 			if ( newstr == NULL )
// 				newstr = "";
// 			free(args[i]);
// 			args[i] = strdup(newstr);
// 		}
// }
#define	is_delim(x) ((x)==' '|| (x)=='\t' || (x)=='\0')

char * varsub(char * args)
{
	int	i = 0;
	char	*newstr, *replace_str;
	FLEXSTR s;
	FLEXSTR to_sub;
// 	char * str;
    fs_init(&s, 0);
    fs_init(&to_sub, 0);
    
    while (args[i] != '\0')
    {
        // go until we reach the start of a var
        while(args[i] != '$' && args[i] != '\0')
        {
            fs_addch(&s, args[i]);
            i++;
        }
        
        i++;        //trim off '$'
        
        if(isdigit((int) args[i]))
        {
            printf("var cannot begin with digit\n");
            continue;
        }
        
        // go until we reach the END of a var
//        while(! is_delim(args[i]) )
        while(isalnum(args[i]) || args[i] == '_')
        {
            fs_addch(&to_sub, args[i]);
            i++;
        }
        
        fs_addch(&to_sub, '\0');
        newstr = fs_getstr(&to_sub);
//         printf("string is %s\n", newstr);
        
//         if(no string was read in...)
        replace_str = VLlookup(newstr);
        if( replace_str == NULL )
            replace_str = "";
        
        fs_free(&to_sub);   //do with varlib function
        fs_addstr(&s, replace_str);
        
        fs_addch(&s, ' ');
        
    }

    char * return_str = fs_getstr(&s);
    fs_free(&s);
    return return_str;
/*
		FLEXSTR	s;
		char    *str;

		fs_init(&s,0);		// initialize the string
		fs_addch(&s,'a');		// append a char
		fs_addch(&s,'b');		// and another char
		fs_addstr(&s, "xyz");		// and a bunch of chars
		str = fs_getstr(&s);		// retrieve string
		printf("string is %s", str);	// print it
		free(str);			// it was malloc-ed
		fs_free(&s);			// deallocate space in FLEXSTR
*/

}