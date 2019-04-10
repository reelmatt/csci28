#ifndef	BUILTIN_H
#define	BUILTIN_H

int is_builtin(char **args, int *resultp);
int is_assign_var(char *cmd, int *resultp);
int is_list_vars(char *cmd, int *resultp);
void varsub(char **args);
int assign(char *);
int okname(char *);
int is_export(char **, int *);

#endif
