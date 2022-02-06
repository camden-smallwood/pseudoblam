#pragma once

/* ---------- prototypes/RENDER.C */

void render_initialize(void);
void render_dispose(void);
void render_handle_screen_resize(int width, int height);
void render_update(float delta_time);
