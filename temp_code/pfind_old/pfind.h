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
int check_entry(char *, int, char *, char *, mode_t);
int check_type(int, mode_t);

int get_type(char);
void get_option(char **, char **, int *);
void get_path(char **, char **, char **, int *);

/* function delcarations for pfind-errors.c */
void syntax_error();
void type_error(char *, char *);
void file_error(char *);

/* function delcarations for pfind-helpers.c */
struct stat * new_stat();
char * construct_path(char *, char *);