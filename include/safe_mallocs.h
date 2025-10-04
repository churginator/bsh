#ifndef SAFE_MALLOCS_H
#define SAFE_MALLOCS_H

#include <stddef.h>

void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);

#endif //SAFE_MALLOCS_H