#pragma once
#include <limits.h>
#include <stddef.h>

/* ---------- macros */

#define BIT(bit_index) (1u << (bit_index))
#define TEST_BIT(bits, bit_index) ((bits) & BIT(bit_index))
#define SET_BIT(bits, bit_index, value) ((bits) = ((value) ? ((bits) | BIT(bit_index)) : ((bits) ^ BIT(bit_index))))

#define BIT_VECTOR_LENGTH_IN_WORDS(bit_count) (((bit_count) + (WORD_BIT - 1)) / WORD_BIT)
#define BIT_VECTOR_WORD_INDEX(bit_index) ((bit_index) / WORD_BIT)
#define BIT_VECTOR_WORD_BIT_INDEX(bit_index) ((bit_index) % WORD_BIT)
#define BIT_VECTOR_TEST_BIT(bits, bit_index) TEST_BIT((bits)[BIT_VECTOR_WORD_INDEX(bit_index)], BIT_VECTOR_WORD_BIT_INDEX(bit_index))
#define BIT_VECTOR_SET_BIT(bits, bit_index, value) SET_BIT((bits)[BIT_VECTOR_WORD_INDEX(bit_index)], BIT_VECTOR_WORD_BIT_INDEX(bit_index), (value))

/* ---------- prototypes/COMMON.C */

void *mempush(
    int *out_count,
    void **out_address,
    void *address,
    size_t size,
    void *(*realloc_function)(void *, size_t));
