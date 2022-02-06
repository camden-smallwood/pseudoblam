#pragma once
#include <stdbool.h>

/* ---------- prototypes/INPUT.C */

void input_initialize(void);
void input_dispose(void);

bool input_is_key_down(int key_code);
void input_set_key_down(int key_code, bool down);

void input_get_mouse_motion(float *out_x, float *out_y);
void input_set_mouse_motion(float x, float y);
