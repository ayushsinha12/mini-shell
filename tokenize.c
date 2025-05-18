#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 1024  
#define MAX_TOKENS 256       

// The list of special characters that should be treated as their own token
const char *SPECIAL_CHARS = "()< >;|";

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

        // Stores everything in one token if it is inside quotes
        if (in_quotes) {
            token[token_index++] = c;
            continue;
        }

        // Takes care of the white space outside the quotations
        if (isspace(c)) {
            if (token_index > 0) {
                token[token_index] = '\0';
                tokens[token_count] = malloc(strlen(token) + 1);
                strcpy(tokens[token_count++], token);
                token_index = 0;
            }
            continue;
        }

        // Treats the special characters as their own token
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

int main() {
    char input[MAX_INPUT_SIZE];

    // Reads the input line
    if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
        return 1;  // Error reading input
    }

    // Removes the ending new line if it is there
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    // Prints the tokenized input
    char **tokens = tokenize(input);
    if (tokens == NULL) {
        return 1;
    }

    for (int i = 0; tokens[i] != NULL; i++) {
        printf("%s\n", tokens[i]);
        free(tokens[i]);
    }

    free(tokens);
    return 0;
}