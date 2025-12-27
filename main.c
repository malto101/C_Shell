#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include "./builtin_cmds/main_cmd.h"
#include "./main.h"


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


// Reads a line from input, handling line continuation with backslash at end of line
char *dsh_read_line(void)
{
    static struct termios orig_termios;
    struct termios raw;
    int bufsize = INITIAL_BUFFER_SIZE;
    int position = 0;
    int cursor = 0;
    char *buffer = malloc(bufsize * sizeof(char));
    int c;

    if (!buffer)
    {
        fprintf(stderr, "dsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    memset(buffer, 0, bufsize);

    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    setbuf(stdout, NULL);

    while (1)
    {
        read(STDIN_FILENO, &c, 1);

        if (c == ESCAPE) {
            read(STDIN_FILENO, &c, 1);
            if (c == '[') {
                read(STDIN_FILENO, &c, 1);
                if (c == 'C') { // right arrow
                    if (cursor < position) {
                        cursor++;
                        printf(CURSOR_RIGHT);
                    }
                } else if (c == 'D') { // left arrow
                    if (cursor > 0) {
                        cursor--;
                        printf(CURSOR_LEFT);
                    }
                }
            }
        } else if (c == BACKSPACE) {
            if (cursor > 0) {
                memmove(&buffer[cursor-1], &buffer[cursor], position - cursor);
                position--;
                cursor--;
                buffer[position] = '\0';
                printf(CARRIAGE_RETURN CLEAR_ENTIRE_LINE "╰-> %s", buffer);
                printf("\x1b[%dD" CURSOR_RIGHT_ONE, position - cursor);
            }
        } else if (c == ENTER_CR || c == ENTER_LF) {
            if (position > 0 && buffer[position - 1] == '\\') {
                position--;
                cursor--;
                buffer[position] = '\0';
                printf(CARRIAGE_RETURN CLEAR_ENTIRE_LINE "╰-> %s", buffer);
                printf("\x1b[%dD" CURSOR_RIGHT_ONE, position - cursor);
            } else {
                buffer[position] = '\0';
                printf("\n");
                break;
            }
        } else if (c >= PRINTABLE_MIN && c <= PRINTABLE_MAX) {
            if (position + 1 >= bufsize) {
                bufsize += BUFFER_INCREMENT;
                buffer = realloc(buffer, bufsize);
                if (!buffer)
                {
                    fprintf(stderr, "dsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
            memmove(&buffer[cursor+1], &buffer[cursor], position - cursor);
            buffer[cursor] = c;
            position++;
            cursor++;
            buffer[position] = '\0';
            printf("%c", c);
            if (cursor < position) {
                printf(CLEAR_FROM_CURSOR);
                printf("%s", &buffer[cursor]);
                printf("\x1b[%dD", position - cursor);
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    return buffer;
}


#define DSH_TOK_DELIM " \t\r\n\a"

// Splits the line into tokens, supporting double quotes for grouping arguments.
// Backslash escaping removed; backslash is now used for line continuation in input reading.
char **dsh_split_line(char *line)
{
    int bufsize = INITIAL_BUFFER_SIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token = NULL;
    int token_size = 0;
    int token_capacity = INITIAL_TOKEN_SIZE;
    // Flag to track if we are inside double quotes
    bool in_quotes = false;
    int i = 0;

    if (!tokens) {
        fprintf(stderr, "dsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = malloc(token_capacity * sizeof(char));
    if (!token) {
        fprintf(stderr, "dsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (line[i] != '\0') {
        char current_char = line[i];

        if (current_char == '"') {
            // Toggle quote state: entering or exiting quoted string
            in_quotes = !in_quotes;
        } else if ((current_char == ' ' || current_char == '\t' || current_char == '\r' || current_char == '\n' || current_char == '\a') && !in_quotes) {
            // End of token if not in quotes (spaces separate arguments)
            if (token_size > 0) {
                token[token_size] = '\0';
                tokens[position++] = token;

                if (position >= bufsize) {
                    bufsize += BUFFER_INCREMENT;
                    tokens = realloc(tokens, bufsize * sizeof(char*));
                    if (!tokens) {
                        fprintf(stderr, "dsh: allocation error\n");
                        exit(EXIT_FAILURE);
                    }
                }

                // Start new token
                token_capacity = INITIAL_TOKEN_SIZE;
                token = malloc(token_capacity * sizeof(char));
                if (!token) {
                    fprintf(stderr, "dsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
                token_size = 0;
            }
        } else {
            // Add character to current token
            if (token_size >= token_capacity - 1) {
                token_capacity *= TOKEN_INCREMENT;
                token = realloc(token, token_capacity * sizeof(char));
                if (!token) {
                    fprintf(stderr, "dsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
            token[token_size++] = current_char;
        }

        i++;
    }

    // Add final token if any
    if (token_size > 0) {
        token[token_size] = '\0';
        tokens[position++] = token;
    } else {
        free(token);
    }

    tokens[position] = NULL;
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
