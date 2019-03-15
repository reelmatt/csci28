#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "pfind.h"


// char * construct_path(char *parent, char *child)
// {
// 	char *newstr = malloc(PATHLEN);
// 	
// 	//Get memory for newstr and check
// 	if (newstr == NULL)
// 		fatal("", "memory error: not enough memory to create new string\n", "");
// 	
// 	if (strcmp(parent, child) == 0 || strcmp(parent, "") == 0)
// 		snprintf(newstr, PATHLEN, "%s", parent);
// 	else
// 		snprintf(newstr, PATHLEN, "%s/%s", parent, child);
// 	
// 	return newstr;
// }
// 
// /*
//  * new_stat()
//  * Purpose: allocate memory for a stat struct, and check for errors
//  *  Return: a pointer to the newly allocated stat struct
//  *  Errors: if malloc fails, call fatal() to quit the program with an
//  *			error message.
//  */
// struct stat * new_stat()
// {
// 	struct stat *new_stat = malloc(sizeof(struct stat));
// 	
// 	if (new_stat == NULL)
// 		fatal("", "memory error: could not allocate a stat struct\n", "");
// 	
// 	return new_stat;
// }