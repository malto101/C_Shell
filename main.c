#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "./builtin_cmds/main_cmd.h"

void dsh_loop(void);
char *dsh_read_line(void);
char **dsh_split_line(char *line);
int dsh_execute(char **args);
int dsh_launch(char **args);


int main(int argc, char **argv)
{
    dsh_loop();
    return EXIT_FAILURE;
}

void dsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("");
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("╭-| %s\n", cwd);
        }
        printf("╰-> ");
        line = dsh_read_line();
        args = dsh_split_line(line);
        status = dsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

#define DSH_RL_BUFSIZE 1024

char *dsh_read_line(void)
{
    int bufsize = DSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char)* bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "dsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        c = getchar();
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position] = c;
        }
        position++;

        if (position > bufsize)
        {
            bufsize += DSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "dsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


#define DSH_TOK_BUFSIZE 64
#define DSH_TOK_DELIM " \t\r\n\a"

char **dsh_split_line(char *line)
{
    int bufsize = DSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens)
    {
        fprintf(stderr, "dsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DSH_TOK_DELIM);
    while(token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += DSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens)
            {
                fprintf(stderr,"dsh: allocation error");
                exit(EXIT_FAILURE);
            }
        }
        /*
         * Continues tokenizing the input string using strtok with NULL as the first argument,
         * which resumes parsing from where the previous call left off. This line is added to
         * extract the next token from the command line input, allowing the shell to parse
         * multiple arguments or commands separated by the delimiter DSH_TOK_DELIM (likely
         * whitespace or similar). This is essential for breaking down user input into
         * individual components for further processing in the shell implementation.
         */
        token = strtok(NULL, DSH_TOK_DELIM);

    }

    tokens[position] =NULL;
    return tokens;
}


int dsh_launch(char **args)
{
    pid_t pid;
    int status;
    pid = fork();
    if (pid==0)
    {
        //child process
        if (execvp(args[0],args) == -1)
        {
            perror("dsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) 
    {
        perror("dsh");
    }
    else {
        do {
            waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) &&!WIFSIGNALED(status));
    }
    return 1;
}

int dsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < dsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return dsh_launch(args);
}
