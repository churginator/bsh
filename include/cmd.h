#ifndef COMMAND_H
#define COMMAND_H

#include <tree.h>

int load_tree(tree_t *input);
int execute_chain(pid_t to_watch);

#endif // COMMAND_H