/*
SHELL.C
    Main application code.
*/

/* ---------- headers */

#include <stdint.h>
#include <stdlib.h>

#include <SDL.h>

#include "input.h"
#include "lights.h"
#include "models.h"
#include "shaders.h"
#include "text.h"
#include "render.h"

/* ---------- private types */

struct shell_component
{
    const char *name;
    void(*initialize)(void);
    void(*dispose)(void);
    void(*handle_screen_resize)(int width, int height);
    void(*update)(float delta_ticks);
};

/* ---------- private constants */

static const struct shell_component shell_components[] =
{
    {
        "input",
        input_initialize,
        input_dispose,
        NULL,
        NULL,
    },
    {
        "lights",
        lights_initialize,
        lights_dispose,
        NULL,
        NULL,
    },
    {
        "models",
        models_initialize,
        models_dispose,
        NULL,
        NULL,
    },
    {
        "shaders",
        shaders_initialize,
        shaders_dispose,
        NULL,
        NULL,
    },
    {
        "text",
        text_initialize,
        text_dispose,
        NULL,
        NULL,
    },
    {
        "render",
        render_initialize,
        render_dispose,
        render_handle_screen_resize,
        render_update,
    },
};

enum
{
    NUMBER_OF_SHELL_COMPONENTS = sizeof(shell_components) / sizeof(struct shell_component)
};

/* ---------- private variables */

struct
{
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
    
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

    const int screen_width = 1280;
    const int screen_height = 720;

    shell_globals.window = SDL_CreateWindow("asdf", 0, 0, screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    shell_globals.gl_context = SDL_GL_CreateContext(shell_globals.window);
    
    shell_globals.frame_rate = 120;

    SDL_GL_SetSwapInterval(0);

    SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    for (int i = 0; i < NUMBER_OF_SHELL_COMPONENTS; i++)
    {
        if (shell_components[i].initialize)
        {
            printf("initializing %s component...\n", shell_components[i].name);
            shell_components[i].initialize();
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
            printf("disposing %s component...\n", shell_components[i].name);
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
    // printf("elapsed frame time: %lf\n", delta_ticks);

    input_set_mouse_motion(0.0f, 0.0f);
    
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_WINDOWEVENT:
            switch(event.window.type)
            {
            case SDL_WINDOWEVENT_RESIZED:
                shell_handle_screen_resize(event.window.data1, event.window.data2);
                break;
            }
            break;
        
        case SDL_KEYDOWN:
            input_set_key_down(event.key.keysym.scancode, true);
            break;
        
        case SDL_KEYUP:
            input_set_key_down(event.key.keysym.scancode, false);
            break;
        
        case SDL_MOUSEMOTION:
            input_set_mouse_motion(event.motion.xrel, event.motion.yrel);
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

    while (((double)(SDL_GetPerformanceCounter() - frame_start_time) / (double)SDL_GetPerformanceFrequency()) < (1.0 / (double)shell_globals.frame_rate));

    shell_globals.last_frame_time = frame_start_time;
    shell_globals.frame_count++;
}
