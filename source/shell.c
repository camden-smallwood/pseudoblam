#include <stdlib.h>
#include <SDL.h>
#include "render.h"

/* ---------- private variables */

struct
{
    SDL_Window *window;
    SDL_GLContext gl_context;
    SDL_Event event;
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

    shell_globals.window = SDL_CreateWindow("asdf", 0, 0, 1280, 720, SDL_WINDOW_OPENGL);
    shell_globals.gl_context = SDL_GL_CreateContext(shell_globals.window);

    render_initialize();
}

static inline void shell_dispose(void)
{
    render_dispose();

    SDL_GL_DeleteContext(shell_globals.gl_context);
    SDL_DestroyWindow(shell_globals.window);
    SDL_Quit();

    exit(EXIT_SUCCESS);
}

static inline void shell_update(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            //
            // TODO: handle input events
            //
            
        case SDL_QUIT:
            shell_dispose();
        }
    }

    render_update();

    SDL_GL_SwapWindow(shell_globals.window);
}
