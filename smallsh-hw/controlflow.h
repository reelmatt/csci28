/*
 * controlflow.h
 */
 
 
int is_control_command(char *);
int do_control_command(char **);
int ok_to_execute();

int is_for_loop(char *s);
int do_for_loop(char **args);
int init_for_loop(char **args);

char ** get_for_commands();
char ** get_for_vars();
char * get_for_name();
int is_parsing_for();
int load_for_loop(char *args);
char * get_next_cmd();