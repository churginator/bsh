#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>

#include <builtins.h>

// returns 0 if a builtin was found, 1 otherwise
// the return status of the builtin is placed in n
int handle_builtins(char **input, int *n) {
    if (!strcmp(input[0], "cd")) {
        if (chdir(input[1]) == -1) {
            error(EXIT_SUCCESS, errno, "%s", input[1]);
            *n = 1;
        } else {
            *n = 0;
        }
    } else {
        return 0;
    }

    return 1;
}