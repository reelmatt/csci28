/*
 * ==========================
 *   FILE: ./sttyl.h
 * ==========================
 * Purpose: Search directories and subdirectories for files matching criteria.
 *
 */

#define ERROR 1
#define ON	1
#define OFF 0
#define YES 1
#define NO  0
#define CHAR_MASK 64

//TABLES
struct flags {tcflag_t fl_value; char *fl_name; };
struct cchars {cc_t c_value; char *c_name; };
struct table { char *name; struct flags * table; tcflag_t  * mode; };

 /* 
  * ==========================
  *   tty_tables.c
  * ==========================
  */
struct flags * get_flags(char *);
struct cchars * get_chars();
struct table * get_table();

 /* 
  * ==========================
  *   termfuncs.c
  * ==========================
  */
struct winsize get_term_size();
void get_settings(struct termios *);
int set_settings(struct termios *);
int getbaud(int);