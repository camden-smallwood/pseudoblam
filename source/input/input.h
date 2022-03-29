#pragma once
#include <stdbool.h>

/* ---------- prototypes/INPUT.C */

bool input_is_key_down(int key_code);
void input_get_mouse_motion(int *out_x, int *out_y);
