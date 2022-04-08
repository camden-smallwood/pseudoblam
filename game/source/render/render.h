/*
RENDER.H
    Main rendering declarations.
*/

#pragma once
#include <cglm/cglm.h>

/* ---------- prototypes/RENDER.C */

void render_initialize(void);
void render_dispose(void);
void render_handle_screen_resize(int width, int height);
void render_load_content(void);
void render_update(float delta_ticks);
