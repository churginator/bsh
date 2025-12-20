#include <tokens.h>
#include <tree.h>
#include <stdbool.h>
#include <safe_mallocs.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>
#include <stdio.h>

lexer_t *create_lexer() {
    return safe_malloc(sizeof(lexer_t));
}

static inline bool b_isspace(char i) {
    return (i == ' ' || i == '\t');
}

static inline char reverse(lexer_t *lexer) {
    lexer->current_index++;
    return lexer->program[lexer->current_index];
}

static inline char set_char(lexer_t *lexer, char c) {
    lexer->program[lexer->current_index] = c;
    return c;
}

static inline char next_char(lexer_t *lexer) {
    int difference;

    if (lexer->direction == LEFT) {
        difference = 1;
    } else {
        difference = -1;
    }

    if (lexer->current_index == 0 && lexer->direction == RIGHT) {
        return '\0';
    }

    lexer->current_index += difference;
    return lexer->program[lexer->current_index];
}

static inline char *skip_whitespace(char *input) {
    while (*input != '\0' && b_isspace(*input)) {
        input++;
    }

    return input;
}

// recursive function to parse a line of input
// quite possibly the worst code i have ever written
static node_t *lex_internal(lexer_t *lexer, node_t *parent) {
    node_t *output;
    node_t *next;
    char n;
    char f;
    bool symbol_found;
    bool symbols_exist;
    bool digraph_found;

    output = NULL;
    next = NULL;
    symbols_exist = false;

    do {
        n = next_char(lexer);
        symbol_found = true;
        digraph_found = false;

        char e = next_char(lexer);
        if (e == '\\') {
            continue;
        } else if (e != '\0') {
            reverse(lexer);
        }

        switch (n) {
        case '&':
            f = next_char(lexer);
            if (f == '&') {
                digraph_found = true;
                next = create_node(parent);
                next->type = AND;
                break;
            }
            if (f != '\0') {
                reverse(lexer);
            }
            next = create_node(parent);
            next->type = BACKGROUND;
            break;
        case '|':
            f = next_char(lexer);
            if (f == '|') {
                digraph_found = true;
                next = create_node(parent);
                next->type = OR;
                break;
            }

            if (f != '\0') {
                reverse(lexer);
            }
            next = create_node(parent);
            next->type = PIPE;
            break;
        case '>':
            next = create_node(parent);
            next->type = REDIRECTION;
            break;
        case ';':
            next = create_node(parent);
            next->type = SEMICOLON;
            break;
        default:
            symbol_found = false;
            break;
        }

        // we can safely assume that next is not a nullptr
        if (symbol_found) {
            size_t extra = 1; // better documentation never
            leaf_t *next_right;

            symbols_exist = true;

            set_char(lexer, '\0');

            if (b_isspace(next_char(lexer))) {
                set_char(lexer, '\0');
            }

            if (digraph_found) {
                extra++;
                set_char(lexer, '\0');
            }

            // set the right to what we've already parsed (guaranteed no symbols there), and recursively parse the left side
            next_right = create_leaf(next);
            next_right->command = skip_whitespace(lexer->program + lexer->current_index + 1 + extra);

            next->meta.right = (node_t *) next_right;
            next->meta.left = lex_internal(lexer, next);

            output = next;
        }

    } while (n != '\0');

    // there are no special symbols, go ahead and set the output node to a leaf and populate it
    if (!symbols_exist) {
        leaf_t *output_l = create_leaf(parent);

        output_l->command = lexer->program;

        free(output);
        output = (node_t *) output_l;
    }

    return output;
}

tree_t *lex(char *line) {
    node_t *output_root;

    lexer_t *lexer_curr = create_lexer();

    lexer_curr->program = line;
    lexer_curr->previous_index = 0;
    lexer_curr->current_index = strlen(line);
    lexer_curr->direction = RIGHT;

    output_root = lex_internal(lexer_curr, NULL);

    free(lexer_curr);

    return init_tree(output_root);
}