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
    uint64_t start_time;
    uint64_t last_update_time;
    uint64_t last_fps_display_time;
    uint64_t total_frames;
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
    
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

    SDL_GL_SetSwapInterval(0);

    const int screen_width = 1280;
    const int screen_height = 720;

    shell_globals.window = SDL_CreateWindow("asdf", 0, 0, screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    shell_globals.gl_context = SDL_GL_CreateContext(shell_globals.window);

    SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    input_initialize();
    
    render_initialize();
    render_handle_screen_resize(screen_width, screen_height);
    
    shell_globals.start_time = SDL_GetTicks64();
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

    if (!shell_globals.last_update_time)
    {
        shell_globals.last_update_time = ticks;
    }

    if ((ticks - shell_globals.last_fps_display_time) >= 1000)
    {
        float fps = shell_globals.total_frames / ((ticks - shell_globals.start_time) / 1000.f);
        if(fps > 2000000)
            fps = 0;
        
        char fps_string[256];
        snprintf(fps_string, sizeof(fps_string), "fps: %f", fps);
        
        SDL_SetWindowTitle(shell_globals.window, fps_string);

        shell_globals.last_fps_display_time = ticks;
    }

    float delta_time = (ticks - shell_globals.last_update_time) / 1000.0f;

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
        
        case SDL_MOUSEMOTION:
            input_set_mouse_motion(event.motion.xrel * 0.01f, event.motion.yrel * 0.01f);
            break;
            
        case SDL_QUIT:
            shell_dispose();
        }
    }

    render_update(delta_time);

    SDL_GL_SwapWindow(shell_globals.window);

    shell_globals.last_update_time = ticks;
    shell_globals.total_frames++;
}
