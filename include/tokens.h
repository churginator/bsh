#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    NOTHING,
    COMMAND,
    REDIRECTION,
    PIPE,
    VARIABLE,
    SEMICOLON,
    AND,
    OR,
    BACKGROUND
} token_t;

/*
static char *token_names[] = {
    "Nothing",
    "Command",
    "Redirection",
    "Pipe",
    "Variable",
    "Semicolon",
    "And",
    "Or",
    "Background"
};
*/

#endif // TOKENS_H