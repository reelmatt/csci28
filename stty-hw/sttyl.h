/*
 * ==========================
 *   FILE: ./sttyl.h
 * ==========================
 * Purpose: Search directories and subdirectories for files matching criteria.
 *
 */



//TABLES
struct flaginfo {tcflag_t fl_value; char *fl_name; };
struct cflaginfo {cc_t c_value; char *c_name; };
//struct flags { struct flaginfo table; };

//typedef struct flaginfo 

// struct flaginfo input_flags[];
// struct flaginfo output_flags[];
// struct flaginfo control_flags[];
// struct flaginfo local_flags[];
/*
struct flaginfo input_flags[];
struct flaginfo output_flags[];
struct flaginfo control_flags[];
struct flaginfo local_flags[];
*/

//struct flaginfo * get_input_flags();
 
 /* 
  * ==========================
  *   termfuncs.
  * ==========================
  */
struct winsize get_term_size();
void get_settings(struct termios *);
void set_settings(struct termios *);