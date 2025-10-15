#include <tokens.h>
#include <tree.h>
#include <safe_mallocs.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

node_t *create_node(node_t *parent) {
    node_t *output;

    output = safe_malloc(sizeof(node_t));
    output->meta.parent = parent;
    output->meta.left = NULL;
    output->meta.right = NULL;

    return output;
}

leaf_t *create_leaf(node_t *parent) {
    leaf_t *output;

    output = safe_malloc(sizeof(leaf_t));
    memset(output, 0, sizeof(leaf_t));

    output->meta.parent = parent;
    output->type = COMMAND;
    output->stdin = 0;
    output->stdout = 1;
    output->stderr = 2;

    return output;
}

tree_t *init_tree(node_t *root) {
    tree_t *output;

    output = safe_malloc(sizeof(tree_t));
    output->root = root;

    return output;
}

static inline bool is_leaf(node_t *input) {
    if (input == NULL) return true;

    if (input->type == COMMAND) {
        return true;
    } else {
        return false;
    }
}

static node_t *kill_node(node_t *who) {
    node_t *output;

    output = who->meta.parent;

    // killing leaves is handled by kill_leaf()
    if (who->type != COMMAND) {
        free(who);
    }

    return output;
}

inline int kill_leaf(leaf_t *who) {
    if (who != NULL) {
        free(who->argv);
    }

    free(who);
    return 0;
}

static node_t *traverse(node_t *start) {
    node_t *curr, *output;

    if (start == NULL) return NULL;

    curr = start;

    if (curr->meta.left != NULL) {
        output = curr->meta.left;
        curr->meta.left = NULL;
    } else if (curr->meta.right != NULL) {
        output = curr->meta.right;
        curr->meta.right = NULL;
    } else {
        output = kill_node(curr);
    }

    return output;
}

node_t *next_node(node_t *start) {
    node_t *curr;

    curr = start;
    
    do {
        curr = traverse(curr);
    } while (curr != NULL && curr->meta.left == NULL && curr->meta.right == NULL);

    return curr;
}

leaf_t *next_leaf(node_t *start) {
    node_t *curr;

    curr = start;

    do {
        curr = traverse(curr);
    } while (!is_leaf(curr));

    if (curr == start) {
        curr = NULL;
    }

    return (leaf_t *) curr;
}

void delete_recursive(node_t *who) {
    node_t *victim, *next;

    victim = who;
    while (victim != NULL) {
        if (victim->meta.left != NULL) {
            next = victim->meta.left;
            victim->meta.left = NULL;
        } else if (victim->meta.right != NULL) {
            next = victim->meta.right;
            victim->meta.right = NULL;
        } else {
            next = victim->meta.parent;
            if (victim->type == COMMAND) {
                kill_leaf((leaf_t *) victim);
            } else {
                kill_node(victim);
            }
        }

        victim = next;
    }
}