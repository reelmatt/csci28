#ifndef	BUILTIN_H
#define	BUILTIN_H

int is_builtin(char **args, int *resultp);
int is_assign_var(char *cmd, int *resultp);
int is_list_vars(char *cmd, int *resultp);
int is_export(char **, int *);

int is_exit(char **args, int *resultp);
int is_cd(char **args, int *resultp);
int is_read(char **args, int *resultp);

char * varsub(char *args);
// void varsub(char **args);
int assign(char *);
int okname(char *);

#endif
