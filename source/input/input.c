#include "common/common.h"
#include "input/input.h"
#include <SDL_scancode.h>

/* ---------- private variables */

struct
{
    int mouse_motion_x;
    int mouse_motion_y;
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

void input_get_mouse_motion(int *out_x, int *out_y)
{
    if (out_x)
        *out_x = input_globals.mouse_motion_x;
    
    if (out_y)
        *out_y = input_globals.mouse_motion_y;
}

void input_set_mouse_motion(int x, int y)
{
    input_globals.mouse_motion_x = x;
    input_globals.mouse_motion_y = y;
}
