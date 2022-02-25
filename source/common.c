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

void mempush_multiple(
    int *out_count,
    int address_count,
    void **out_addresses,
    void **addresses,
    size_t *sizes,
    void *(*realloc_function)(void *, size_t))
{
    assert(out_count);
    assert(address_count);
    assert(out_addresses);
    assert(addresses);
    assert(sizes);
    assert(realloc_function);

    int index = (*out_count)++;
    assert(index < *out_count);

    for (int i = 0; i < address_count; i++)
    {
        out_addresses[i] = realloc_function(out_addresses[i], (*out_count) * sizes[i]);
        assert(out_addresses[i]);

        memcpy((char *)out_addresses[i] + (index * sizes[i]), addresses[i], sizes[i]);
    }
}
