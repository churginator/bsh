#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>

#include <builtins.h>

// returns 0 if a builtin was found, 1 otherwise
// the return status of the builtin is placed in n
int handle_builtins(char **argv, int *n) {
    if (!strcmp(argv[0], "cd")) {
        if (chdir(argv[1]) == -1) {
            error(EXIT_SUCCESS, errno, "%s", argv[1]);
            *n = 1;
        } else {
            *n = 0;
        }
    } else if (!strcmp(argv[0], "exit")) {
        // if this returns somehow im killing myself
        exit(0);
    } else {
        return 0;
    }

    return 1;
}