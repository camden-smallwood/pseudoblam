#include "common.h"
#include "input.h"
#include <SDL_scancode.h>

/* ---------- private variables */

struct
{
    unsigned int key_bit_vector[BIT_VECTOR_LENGTH_IN_WORDS(SDL_NUM_SCANCODES)];
} static input_globals;

/* ---------- public code */

void input_initialize(void)
{
    memset(&input_globals, 0, sizeof(input_globals));
}

void input_dispose(void)
{
    // TODO
}

bool input_is_key_down(int key_code)
{
    return BIT_VECTOR_TEST_BIT(input_globals.key_bit_vector, key_code);
}

void input_set_key_down(int key_code, bool down)
{
    BIT_VECTOR_SET_BIT(input_globals.key_bit_vector, key_code, down);
}
