/*
SHELL.C
    Main application code.
*/

/* ---------- headers */

#include <stdint.h>
#include <stdlib.h>

#include <SDL.h>

#include "common/common.h"
#include "input/input.h"
#include "models/models.h"
#include "objects/objects.h"
#include "rasterizer/rasterizer_shaders.h"
#include "objects/lights.h"
#include "game/game.h"
#include "render/render.h"

/* ---------- private types */

struct shell_component
{
    const char *name;
    void(*initialize)(void);
    void(*dispose)(void);
    void(*handle_screen_resize)(int width, int height);
    void(*load_content)(void);
    void(*update)(float delta_ticks);
};

/* ---------- private constants */

static const struct shell_component shell_components[] =
{
    {
        "lights",
        lights_initialize,
        lights_dispose,
        NULL,
        NULL,
        NULL,
    },
    {
        "models",
        models_initialize,
        models_dispose,
        NULL,
        NULL,
        NULL,
    },
    {
        "objects",
        objects_initialize,
        objects_dispose,
        NULL,
        NULL,
        objects_update,
    },
    {
        "shaders",
        shaders_initialize,
        shaders_dispose,
        NULL,
        NULL,
        NULL,
    },
    {
        "game",
        game_initialize,
        game_dispose,
        game_handle_screen_resize,
        game_load_content,
        game_update,
    },
    {
        "render",
        render_initialize,
        render_dispose,
        render_handle_screen_resize,
        render_load_content,
        render_update,
    },
};

enum
{
    NUMBER_OF_SHELL_COMPONENTS = sizeof(shell_components) / sizeof(struct shell_component)
};

enum shell_flags
{
    _shell_capture_mouse_bit,
    NUMBER_OF_SHELL_FLAGS
};

/* ---------- private variables */

struct
{
    unsigned int flags;
    SDL_Window *window;
    SDL_GLContext gl_context;
    SDL_Event event;
    uint64_t frame_rate;
    uint64_t frame_count;
    uint64_t last_frame_time;
    uint64_t last_fps_display_time;
} static shell_globals;

/* ---------- private prototypes */

static inline void shell_initialize(void);
static inline void shell_dispose(void);
static inline void shell_handle_screen_resize(int width, int height);
static inline void shell_update(void);

/* ---------- public code */

void shell_get_window_size(int *out_width, int *out_height)
{
    SDL_GetWindowSize(shell_globals.window, out_width, out_height);
}

int main(void)
{
    shell_initialize();

    for (;;)
    {
        shell_update();
    }
}

/* ---------- private code */

static inline void shell_initialize(void)
{
    memset(&shell_globals, 0, sizeof(shell_globals));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "ERROR: failed to initialize SDL - %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    const int screen_width = 1280;
    const int screen_height = 720;

    shell_globals.window = SDL_CreateWindow("asdf", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    shell_globals.gl_context = SDL_GL_CreateContext(shell_globals.window);
    
    shell_globals.frame_rate = 60;

    SDL_GL_SetSwapInterval(0);

    SET_BIT(shell_globals.flags, _shell_capture_mouse_bit, 1);

    for (int i = 0; i < NUMBER_OF_SHELL_COMPONENTS; i++)
    {
        if (shell_components[i].initialize)
        {
            shell_components[i].initialize();
        }
    }

    for (int i = 0; i < NUMBER_OF_SHELL_COMPONENTS; i++)
    {
        if (shell_components[i].load_content)
        {
            shell_components[i].load_content();
        }
    }

    shell_handle_screen_resize(screen_width, screen_height);
}

static inline void shell_dispose(void)
{
    for (int i = NUMBER_OF_SHELL_COMPONENTS - 1; i >= 0; i--)
    {
        if (shell_components[i].dispose)
        {
            shell_components[i].dispose();
        }
    }

    SDL_GL_DeleteContext(shell_globals.gl_context);
    SDL_DestroyWindow(shell_globals.window);
    SDL_Quit();

    exit(EXIT_SUCCESS);
}

static inline void shell_handle_screen_resize(int width, int height)
{
    for (int i = 0; i < NUMBER_OF_SHELL_COMPONENTS; i++)
    {
        if (shell_components[i].handle_screen_resize)
        {
            shell_components[i].handle_screen_resize(width, height);
        }
    }
}

static inline void shell_update(void)
{
    SDL_CaptureMouse(TEST_BIT(shell_globals.flags, _shell_capture_mouse_bit) ? SDL_TRUE : SDL_FALSE);
    SDL_SetRelativeMouseMode(TEST_BIT(shell_globals.flags, _shell_capture_mouse_bit) ? SDL_TRUE : SDL_FALSE);

    uint64_t frame_start_time = SDL_GetPerformanceCounter();

    if (!shell_globals.last_frame_time)
    {
        shell_globals.last_frame_time = frame_start_time;
    }

    if (((double)(frame_start_time - shell_globals.last_fps_display_time) / (double)SDL_GetPerformanceFrequency()) >= 1.0)
    {
        char fps_string[256];
        snprintf(fps_string, sizeof(fps_string), "fps: %llu", shell_globals.frame_count);

        SDL_SetWindowTitle(shell_globals.window, fps_string);

        shell_globals.last_fps_display_time = SDL_GetPerformanceCounter();
        shell_globals.frame_count = 0;
    }

    double delta_ticks = ((double)(frame_start_time - shell_globals.last_frame_time) / (double)SDL_GetPerformanceFrequency());

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
                shell_handle_screen_resize(event.window.data1, event.window.data2);
                break;
            }
            break;
        
        case SDL_KEYUP:
            if (event.key.keysym.scancode == SDL_SCANCODE_M)
                SET_BIT(shell_globals.flags, _shell_capture_mouse_bit, !TEST_BIT(shell_globals.flags, _shell_capture_mouse_bit));
            break;
        
        case SDL_QUIT:
            shell_dispose();
        }
    }

    for (int i = 0; i < NUMBER_OF_SHELL_COMPONENTS; i++)
    {
        if (shell_components[i].update)
        {
            shell_components[i].update(delta_ticks);
        }
    }
    
    SDL_GL_SwapWindow(shell_globals.window);

    while (((double)(SDL_GetPerformanceCounter() - frame_start_time) / (double)SDL_GetPerformanceFrequency()) < (1.0 / (double)(shell_globals.frame_rate - 1)));

    shell_globals.last_frame_time = frame_start_time;
    shell_globals.frame_count++;
}
