#include "common/common.h"
#include "input/input.h"
#include <SDL.h>

/* ---------- public code */

bool input_is_key_down(int key_code)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    return keys[key_code] != 0;
}

void input_get_mouse_motion(int *out_x, int *out_y)
{
    SDL_GetRelativeMouseState(out_x, out_y);
}
