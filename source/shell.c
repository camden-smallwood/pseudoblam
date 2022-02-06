#include <stdint.h>
#include <stdlib.h>
#include <SDL.h>
#include "input.h"
#include "render.h"

/* ---------- private variables */

struct
{
    SDL_Window *window;
    SDL_GLContext gl_context;
    SDL_Event event;
    uint64_t last_update;
} static shell_globals;

/* ---------- private prototypes */

static inline void shell_initialize(void);
static inline void shell_dispose(void);
static inline void shell_update(void);

/* ---------- public code */

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

    shell_globals.window = SDL_CreateWindow("asdf", 0, 0, screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    shell_globals.gl_context = SDL_GL_CreateContext(shell_globals.window);

    input_initialize();
    render_initialize();
    render_handle_screen_resize(screen_width, screen_height);
}

static inline void shell_dispose(void)
{
    render_dispose();
    input_dispose();

    SDL_GL_DeleteContext(shell_globals.gl_context);
    SDL_DestroyWindow(shell_globals.window);
    SDL_Quit();

    exit(EXIT_SUCCESS);
}

static inline void shell_update(void)
{
    uint64_t ticks = SDL_GetTicks64();

    if (!shell_globals.last_update)
    {
        shell_globals.last_update = ticks;
    }

    float delta_time = (ticks - shell_globals.last_update) / 1000.0f;

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_WINDOWEVENT:
            switch(event.window.type)
            {
            case SDL_WINDOWEVENT_RESIZED:
                render_handle_screen_resize(event.window.data1, event.window.data2);
                break;
            }
            break;
        
        case SDL_KEYDOWN:
            input_set_key_down(event.key.keysym.scancode, true);
            break;
        
        case SDL_KEYUP:
            input_set_key_down(event.key.keysym.scancode, false);
            break;
            
        case SDL_QUIT:
            shell_dispose();
        }
    }

    render_update(delta_time);

    SDL_GL_SwapWindow(shell_globals.window);

    shell_globals.last_update = ticks;
}
