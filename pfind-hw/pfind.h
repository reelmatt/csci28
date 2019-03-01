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
void searchdir(char *, char *, int);
int recurse_directory(char *, mode_t);
int check_entry(char *, int, char *, char *, char *, mode_t);
int check_type(int, mode_t);

int get_type(char);
void get_option(char **, char **, int *);
void get_path(char **, char **, char **, int *);

/* function delcarations for pfind-errors.c */
void syntax_fatal();
void type_fatal(char *, char *);
void read_fatal(char *, char *);
void fatal(char *, char *, char *);

/* function delcarations for pfind-helpers.c */
struct stat * new_stat();
char * construct_path(char *, char *);