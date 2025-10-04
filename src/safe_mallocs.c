#include <stdlib.h>
#include <error.h>
#include <errno.h>

// safe_malloc: wrapper for malloc which exits on failure
void *safe_malloc(size_t size) {
    void *output;

    output = malloc(size);

    if (output == NULL) {
        error(EXIT_FAILURE, errno, "malloc");
    }

    return output;
}

void *safe_realloc(void *ptr, size_t size) {
    void *output;

    output = realloc(ptr, size);

    if (output == NULL) {
        error(EXIT_FAILURE, errno, "realloc");
    }

    return output;
}