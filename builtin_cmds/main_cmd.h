#ifndef MAIN_CMD_H
#define MAIN_CMD_H

extern int (*builtin_func[]) (char **);
extern char *builtin_str[];
int dsh_num_builtins();

#endif