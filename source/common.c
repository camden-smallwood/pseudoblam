#include <assert.h>
#include <string.h>
#include "common.h"

/* ---------- public code */

void *mempush(
    int *out_count,
    void **out_address,
    void *address,
    size_t size,
    void *(*realloc_function)(void *, size_t))
{
    assert(out_count);
    assert(out_address);
    assert(address);
    assert(size);
    assert(realloc_function);

    int index = (*out_count)++;
    assert(index < *out_count);

    *out_address = realloc_function(*out_address, *out_count * size);
    assert(*out_address);

    return memcpy(((char *)*out_address) + (index * size), address, size);
}
