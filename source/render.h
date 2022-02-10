#pragma once
#include <cglm/cglm.h>

/* ---------- prototypes/RENDER.C */

void render_initialize(void);
void render_dispose(void);
void render_handle_screen_resize(int width, int height);
void render_update(float delta_ticks);

/* ---------- prototypes/RENDER_TEXT.C */

void render_text_initialize(void);
void render_text_dispose(void);
void render_text(const char *text, const vec3 color, const char *font_file_path, unsigned int font_size);
