/*
 * ==========================
 *   FILE: ./pfind.h
 * ==========================
 */

/* CONSTANTS */
#define NO 				0
#define YES 			1
#define PATHLEN			255

/* function declarations for pfind.c */
void searchdir(char *, char *, char);
int recurse_directory(char *, mode_t);
int check_entry(char *, char, char *, char *, mode_t);
int check_type(char c, mode_t item);

char get_type(char);
void get_option(char **, char **, char *);
void get_path(char *, char **);

/* function delcarations for pfind-errors.c */
void usage_fatal();
void type_fatal(char *, char *);
void read_fatal(char *, char *);
void fatal(char *, char *, char *);

/* function delcarations for pfind-helpers.c */
struct stat * new_stat();
char * construct_path(char *, char *);