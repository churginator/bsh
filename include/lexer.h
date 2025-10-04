#ifndef LEXER_H
#define LEXER_H

#include <tree.h>

typedef struct {
    char *program;
    size_t current_index;
    size_t previous_index;
    enum {
        LEFT, RIGHT
    } direction;

} lexer_t;

tree_t *lex(char *line);

#endif // LEXER_H