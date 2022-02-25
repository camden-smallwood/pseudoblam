/*
COMMON.H
    Commonly-used code declarations.
*/

#pragma once
#include <limits.h>
#include <stddef.h>

/* ---------- macros */

#define BIT(bit_index) (1u << (bit_index))
#define TEST_BIT(bits, bit_index) (((bits) & BIT(bit_index)) != 0)
#define SET_BIT(bits, bit_index, value) ((bits) = ((value) ? ((bits) | BIT(bit_index)) : ((bits) ^ BIT(bit_index))))

#define BIT_VECTOR_LENGTH_IN_WORDS(bit_count) (((bit_count) + (WORD_BIT - 1)) / WORD_BIT)
#define BIT_VECTOR_WORD_INDEX(bit_index) ((bit_index) / WORD_BIT)
#define BIT_VECTOR_WORD_BIT_INDEX(bit_index) ((bit_index) % WORD_BIT)
#define BIT_VECTOR_TEST_BIT(bits, bit_index) TEST_BIT((bits)[BIT_VECTOR_WORD_INDEX(bit_index)], BIT_VECTOR_WORD_BIT_INDEX(bit_index))
#define BIT_VECTOR_SET_BIT(bits, bit_index, value) SET_BIT((bits)[BIT_VECTOR_WORD_INDEX(bit_index)], BIT_VECTOR_WORD_BIT_INDEX(bit_index), (value))

/* ---------- prototypes/COMMON.C */

/**
 * Pushes an element to the end of an abstract dynamic array.
 * @param out_count The address of the count of the dynamic array.
 * @param out_address The address of the elements address of the dynamic array.
 * @param address The address of the element to push to the end of the dynamic array.
 * @param size The size of the elements in the dynamic array.
 * @param realloc_function The address of the reallocation function to use.
 * @returns The address of the newly-pushed element inside of the reallocated dynamic array.
 */
void *mempush(
    int *out_count,
    void **out_address,
    void *address,
    size_t size,
    void *(*realloc_function)(void *, size_t));

/**
 * Pushes multiple elements to the end of an abstract dynamic multiarray.
 * 
 * @param array_count The number of arrays in the dynamic multiarray.
 * @param out_count The address of the count of the dynamic multiarray.
 * @param out_addresses The addresses of the elements addresses of each array in the dynamic multiarray.
 * @param in_addresses The addresses of the elements to push to the end of each array in the dynamic multiarray.
 * @param sizes The sizes of the elements in each array in the dynamic multiarray.
 * @param realloc_function The address of the reallocation function to use.
 */
void mempush_multiple(
    int array_count,
    int *out_count,
    void **out_addresses,
    void **in_addresses,
    size_t *sizes,
    void *(*realloc_function)(void *, size_t));
