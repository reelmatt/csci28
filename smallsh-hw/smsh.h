#ifndef	SMSH_H
#define	SMSH_H

/* Put here things that need to be seen by all parts of the program */
void fatal(char *, char *, int);
enum mode { INTERACTIVE, SCRIPTED };

int get_exit();
void set_exit(int status);
void set_for(int);
int get_mode();

#endif