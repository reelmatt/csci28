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
int valid_var(char * var);

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

//VLlookup("HOME");

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
			if ( isdigit((int) args[1]))
			    exit( (int) args[1]);
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

int valid_var(char * var)
{
	int i;
	int len = strlen(var);
	
	if (isdigit(var[0]))
		return false;

	for(i = 0; i < len; i++)
	{
		if( isalnum(var[i]) || var[i] == '_')
			continue;
		else
			return false;
	}
	
	return true;
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

enum states { LEAVE, EXTRACT };

char * varsub(char * args)
{
	int	i = 0;
	char special_str[10] = "\0";
	
	char	*newstr, *replace_str;
	FLEXSTR s;
	FLEXSTR to_sub;
// 	char * str;
    fs_init(&s, 0);


    
    //go char-by-char until end-of-line
    while (args[i] != '\0')
    {
        //if a $ or an escape char '\', handle
        if(args[i] == '\\')
        {
//             printf("ESCAPE char...\n");
             fs_addch(&s, args[++i]);   //add char after escape
             i++;
             continue;
        }
        //var signifier
        else if (args[i] == '$')
        {
            i++;
            if(args[i] == '$')
            {
//                 printf("getpid()\n");
                sprintf(special_str, "%d", getpid());
            }
            else if (args[i] == '?')
            {
//                 printf("get_exit()\n");
                sprintf(special_str, "%d", get_exit());
            }
            else if (isdigit(args[i]))
            {
                fprintf(stderr, "bad var name, cannot begin with digit\n");
                continue;
            }
            else
            {
                fs_init(&to_sub, 0);
//                 printf("EXTRACTING...\n");
//                 printf("first is %c\n", args[i]);
                
                //extract the var
                while(isalnum(args[i]) || args[i] == '_')
                {
//                     printf("%c", args[i]);
                    fs_addch(&to_sub, args[i]);
                    i++;
                }
                fs_addch(&to_sub, '\0'); //null-terminate string
//                 printf("\n");
//                     sub_mode = EXTRACT;
//                 i++;
            }
            
            //something to replace
            newstr = fs_getstr(&to_sub);
//             printf("special_str == %s\n", special_str);
//             printf("newstr == %s\n", newstr);
        
            replace_str = VLlookup(newstr);
            if( replace_str == NULL )
                replace_str = "";
    //         printf("Mode is %d\n", sub_mode);
        
//             printf("replace_str == %s\n", replace_str);
            fs_free(&to_sub);
            fs_addstr(&s, replace_str);
        
        }
        //just regular chars
        else
        {
            fs_addch(&s, args[i]);
            i++;
            continue;
        }
                //         if(no string was read in...)
//                 fs_free(&to_sub);   //do with varlib function
//                 fs_addstr(&s, replace_str);       

//         i++;
//         printf("going for another while loop\n");
    }
    
//     printf("POST-while loop\n");
    fs_addch(&s, '\0');         //null-terminate full string
    char * return_str = fs_getstr(&s);
//     printf("return_str is %s\n", return_str);
    fs_free(&s);
    return return_str;
}


char * varsub2(char * args)
{
	char c, prev;
	FLEXSTR s;
	fs_init(&s, 0);
	int check;
	
	while ( (c = args[0]) )
	{
// 		printf("checking %c\n", c);
		if (c == '\\')
		{
			args++;
			fs_addch(&s, args[0]);	//add escaped-char
		}
		else if (c == '$')
		{
// 			printf("get_replacement\n");
			args++;	//trim the $
			char *newstr = get_replacement(args, &check);
			args += (check - 1);
// 			printf("newstr is %s\n", newstr);

			fs_addstr(&s, newstr);
			//add to full string &s
			
		}
		else if (c == '#' && is_delim(prev) ) //current char is # and prev was whitespace
		{
			//ignore the rest
		}
		else
		{
			fs_addch(&s, c);	//no special, add as-is
		}
		
		prev = c;
		args++;
	}
	
// 	printf("\nending varsub2\n");
	fs_addch(&s, '\0');
	char * return_str = fs_getstr(&s);
	fs_free(&s);
	return return_str;
}


char * get_replacement(char * args, int * len)
{
// 	printf("in get_replacement, current args is:\n\t\t%s\n", args);
// 	printf("a char is %c\n", args[0]);
	FLEXSTR sub;
	fs_init(&sub, 0);
	
	int skipped = 0;
	
	if(isdigit((int) args[0]))
	{
		fprintf(stderr, "bad var name, cannot begin with digit\n");
		return NULL;
	}
	
	while(args[0])
	{
// 		printf("while check: %c\n", args[0]);
		if(isalnum(args[0]) || args[0] == '_')
		{
			fs_addch(&sub, args[0]);
			args++;
		}
		else
		{
			break;
		}
		skipped++;
	}
	
	fs_addch(&sub, '\0');
	
	char * str = VLlookup(fs_getstr(&sub));
	
// 	char * str = fs_getstr(&sub);
	fs_free(&sub);
	
// 	printf("going to return: %s\n", str);
// 	printf("substituted a var of length %d\n", skipped);
	*len = skipped;
	
	return (str) ? str : "";
}
