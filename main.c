#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ash_RL_BUFSIZE 1024

// Function to read a line from standard input

char *ash_read_line(void)
{
    int bufsize = ash_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();

        // If end of file or new line, terminate the string and return the buffer
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        // If the buffer is exceeded, reallocate more space
        if (position >= bufsize)
        {
            bufsize += ash_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "ash: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define ash_TOK_BUFSIZE 64

// Function to split a line into tokens (words)
#define ash_TOK_DELIM " \t\r\n\a"
char **ash_split_line(char *line)
{
    int bufsize = ash_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Tokenize the input line using the defined delimiters
    token = strtok(line, ash_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += ash_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "ash: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, ash_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// Function to launch a program
int ash_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork(); // Fork a new process
    if (pid == 0)
    {
        // Child process: execute the command
        if (execvp(args[0], args) == -1)
        {
            perror("ash");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("ash");
    }
    else
    {
        // Parent process: wait for the child to terminate
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

// Function declarations for built-in shell commands
int ash_cd(char **args);
int ash_help(char **args);
int ash_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"};

int (*builtin_func[])(char **) = {
    &ash_cd,
    &ash_help,
    &ash_exit};

int ash_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
};

// Change directory
int ash_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "ash: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("ash");
        }
    }
    return 1;
};

// Print help information
int ash_help(char **args)
{
    int i;
    printf("Custom Shell in C\n");
    printf("Type arguments and then hit enter\n");
    printf("The following are built-in programs:\n");

    for (i = 0; i < ash_num_builtins(); i++)
    {
        printf(" %s\n", builtin_str[i]);
    }

    return 1;
}

// Exit the shell
int ash_exit(char **args)
{
    return 0;
}

int ash_execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        return 1;
    }

    for (i = 0; i < ash_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return ash_launch(args);
}

// Main loop of the shell
void ash_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("=> ");
        line = ash_read_line();
        args = ash_split_line(line);
        status = ash_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    ash_loop();

    return EXIT_SUCCESS;
}