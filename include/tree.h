#ifndef TREE_H
#define TREE_H

#include <tokens.h>

typedef struct node_t node_t;

typedef struct {
    node_t *parent;
    node_t *left;
    node_t *right;
} head_t;

struct node_t {
    head_t meta;
    token_t type;
};

typedef struct {
    head_t meta;
    token_t type;
    char *command;
    char **argv;
    int stdin;
    int stdout;
    int stderr;
} leaf_t;

typedef struct {
    node_t *root;
} tree_t;

node_t *create_node(node_t *parent);
leaf_t *create_leaf(node_t *parent);
int kill_leaf(leaf_t *who);
leaf_t *next_leaf(node_t *start);
tree_t *init_tree();
void delete_tree(tree_t *who);

#endif // TREE_H