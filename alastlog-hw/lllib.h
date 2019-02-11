/*
 * lllib.h - header file with functions located in lllib.c
 */

int ll_open(char *);
struct lastlog *ll_next();
int ll_close();
int ll_reset(char *);
