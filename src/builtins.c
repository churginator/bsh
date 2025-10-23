#include <string.h>
#include <unistd.h>

#include <builtins.h>

int handle_builtins(char **input, int *n) {
    if (!strcmp(input[0], "cd")) {
        chdir(input[1]);
        *n = 0;
    } else {
        return 0;
    }

    return 1;
}