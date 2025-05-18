#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKENS 256
#define SPECIAL_CHARS "()< >;|"

/* Function Prototypes */
void display_prompt();
void change_directory(char *path);
void run_script(char *filename);
void print_help();
void prev_command(char *prev_cmd);
void execute_pipeline(char **cmd1, char **cmd2, char *input_file, char *output_file);
void execute_command(char **args);
char **tokenize(const char *input);

int main() {
    char input[MAX_INPUT_SIZE];
    char prev_cmd[MAX_INPUT_SIZE] = "";

    printf("Welcome to mini-shell.\n");

    while (1) {
        display_prompt();
        if (!fgets(input, MAX_INPUT_SIZE, stdin)) {
            printf("\nBye bye.\n");
            exit(0);
        }

        // Removes the ending newline
        input[strcspn(input, "\n")] = '\0';

        // Takes care of the previous command
        if (strcmp(input, "prev") == 0) {
            if (strlen(prev_cmd) == 0) {
                printf("prev: no previous command\n");
                continue;
            } else {
                printf("%s\n", prev_cmd);
                strcpy(input, prev_cmd);
            }
        }

        // Store current comand as the previous
        strcpy(prev_cmd, input);

        // Handles the sequencing of commands
        char *commands[MAX_TOKENS];
        int cmd_index = 0;
        char *cmd = strtok(input, ";");
        while (cmd != NULL) {
            commands[cmd_index++] = cmd;
            cmd = strtok(NULL, ";");
        }
        commands[cmd_index] = NULL;

        //Executes each command seperatly
        for (int i = 0; i < cmd_index; i++) {
            char **tokens = tokenize(commands[i]);

            if (tokens == NULL || tokens[0] == NULL) {
                free(tokens);
                continue;  // If there is an empty command it gets ignored
            }

            //Takes care of the built in commands
            if (strcmp(tokens[0], "exit") == 0) {
                printf("Bye bye.\n");
                for (int j = 0; tokens[j] != NULL; j++) {
                    free(tokens[j]);
                }
                free(tokens);
                exit(0);
            } else if (strcmp(tokens[0], "cd") == 0) {
                change_directory(tokens[1]);
            } else if (strcmp(tokens[0], "source") == 0) {
                if (tokens[1] == NULL) {
                    printf("source: missing filename\n");
                } else {
                    run_script(tokens[1]);
                }
            } else if (strcmp(tokens[0], "help") == 0) {
                print_help();
            } else {
                execute_command(tokens);
            }

            // Frees the allocated tokens
            for (int j = 0; tokens[j] != NULL; j++) {
                free(tokens[j]);
            }
            free(tokens);
        }
    }

    return 0;
}

// Displays the shell prompt
void display_prompt() {
    printf("shell $ ");
    fflush(stdout);
}

// Changes the directory
void change_directory(char *path) {
    if (path == NULL) {
        printf("cd: missing argument\n");
    } else if (chdir(path) != 0) {
        perror("cd failed");
    }
}

// Executes the script
void run_script(char *filename) {
    FILE *file = fopen(filename, "r");
    char line[MAX_INPUT_SIZE];

    if (!file) {
        perror("source failed");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        char **tokens = tokenize(line);
        execute_command(tokens);
        
        // Frees allocated tokens
        for (int i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }

    fclose(file);
}

// Prints the previous command
void prev_command(char *prev_cmd) {
    if (strlen(prev_cmd) == 0) {
        printf("prev: no previous command\n");
    } else {
        printf("%s\n", prev_cmd);
    }
}

// Prints help
void print_help() {
    printf("Mini-shell built-in commands:\n");
    printf("cd <dir> - Change directory\n");
    printf("source <file> - Run a script\n");
    printf("prev - Execute the previous command\n");
    printf("help - Show built-in commands\n");
    printf("exit - Exit shell\n");
}

// Handles the piping between two commands and can support input and output redirection 
void execute_pipeline(char **cmd1, char **cmd2, char *input_file, char *output_file) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // The first process which writes to pipe
        if (input_file) {  //Only redirects if the file exists
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        if (execvp(cmd1[0], cmd1) == -1) {
            perror(cmd1[0]);
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // The second process which reads from the pipe
        if (output_file) {  //Only redirects if the file exists
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        if (execvp(cmd2[0], cmd2) == -1) {
            perror(cmd2[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Closes pipes
    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);
}

// Executes the external command
void execute_command(char **args) {
    int pipe_index = -1;
    int in_redirect = -1, out_redirect = -1;
    char *input_file = NULL, *output_file = NULL;

    // Looks for pipes and redirection
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_index = i;
            break;
        } else if (strcmp(args[i], "<") == 0) {
            args[i] = NULL;
            input_file = args[i + 1];
            in_redirect = i;
        } else if (strcmp(args[i], ">") == 0) {
            args[i] = NULL;
            output_file = args[i + 1];
            out_redirect = i;
        }
    }

    // Handles the piping and redirection
    if (pipe_index != -1) {
        args[pipe_index] = NULL; // splits the commands
        char **cmd1 = args;
        char **cmd2 = &args[pipe_index + 1];
        execute_pipeline(cmd1, cmd2, input_file, output_file);
        return;
    }

    // The normal command execution with piping
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
    } else if (pid == 0) {
        /* Handle Input Redirection */
        if (input_file) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Handles the output redirection
        if (output_file) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Executes the command
        if (execvp(args[0], args) == -1) {
            perror(args[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        wait(NULL);
    }
}

// Tokenize the input
char **tokenize(const char *input) {
    char **tokens = malloc(MAX_TOKENS * sizeof(char *));
    if (!tokens) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    char token[MAX_INPUT_SIZE];
    int token_index = 0;
    int token_count = 0;
    int in_quotes = 0;
    char quote_type = '\0';

    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i];

        // Takes care of entering and exiting quoted strings
        if ((c == '"' || c == '\'') && !in_quotes) {
            in_quotes = 1;
            quote_type = c;
            continue;
        } else if (in_quotes && c == quote_type) {
            in_quotes = 0;
            token[token_index] = '\0';
            tokens[token_count] = malloc(strlen(token) + 1);
            strcpy(tokens[token_count++], token);
            token_index = 0;
            continue;
        }

        // Store everything as part of the token if it is inside quotes
        if (in_quotes) {
            token[token_index++] = c;
            continue;
        }

        // Takes care of the whitespace outside of quotation
        if (isspace(c)) {
            if (token_index > 0) {
                token[token_index] = '\0';
                tokens[token_count] = malloc(strlen(token) + 1);
                strcpy(tokens[token_count++], token);
                token_index = 0;
            }
            continue;
        }

       // Uses seperate tokens for special characters
        if (strchr(SPECIAL_CHARS, c) != NULL) {
            if (token_index > 0) {
                token[token_index] = '\0';
                tokens[token_count] = malloc(strlen(token) + 1);
                strcpy(tokens[token_count++], token);
                token_index = 0;
            }
            char special[2] = {c, '\0'};
            tokens[token_count] = malloc(2);
            strcpy(tokens[token_count++], special);
            continue;
        }

        // Takes care of the normal words
        token[token_index++] = c;
    }

    // Print the last token if it even exists
    if (token_index > 0) {
        token[token_index] = '\0';
        tokens[token_count] = malloc(strlen(token) + 1);
        strcpy(tokens[token_count++], token);
    }

    // Makes the token array null, making it destroyed
    tokens[token_count] = NULL;
    return tokens;
}