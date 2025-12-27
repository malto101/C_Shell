#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int dsh_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "dsh:expected argument to \"cd\"\n");
    }
    else {
        if (chdir(args[1]) != 0)
        {
            perror("dsh");
        }
    }
    return 1;

}