/*
GAME.H
    Game management declarations.
*/

#pragma once

#include "common/common.h"
#include "camera/camera.h"

/* ---------- prototypes/GAME.C */

struct camera_data *game_get_player_camera(void);

void game_initialize(void);
void game_dispose(void);
void game_handle_screen_resize(int width, int height);
void game_load_content(void);
void game_update(float delta_ticks);
