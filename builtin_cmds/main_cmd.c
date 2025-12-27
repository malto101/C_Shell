#include <stdio.h>
#include "./dsh_cd.h"
#include "./dsh_exit.h"

char *builtin_str[] = {
"cd",
"help",
"exit"
};

int dsh_num_builtins()
{
    return sizeof(builtin_str) /sizeof(char *);
}

int dsh_help(char **args)
{
  int i;
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < dsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int (*builtin_func[]) (char **) = {
    &dsh_cd,
    &dsh_help,
    &dsh_exit
};

