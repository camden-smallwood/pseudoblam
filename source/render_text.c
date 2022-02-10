#include <stdio.h>

#include <GL/glew.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "render.h"

/* ---------- private prototypes */

// static GLuint render_text_as_texture2d(
//     const char *text,
//     const char *font_file_path,
//     SDL_Color font_color,
//     int font_size,
//     int *out_width,
//     int *out_height);

/* ---------- public code */

void render_text_initialize(
    void)
{
    // TODO
}

void render_text_dispose(
    void)
{
    // TODO
}

// void render_text(
//     const char *text,
//     const vec3 color,
//     const char *font_file_path,
//     unsigned int font_size)
// {
//     // TODO
// }

/* ---------- private code */

// static GLuint render_text_as_texture2d(
//     const char *text,
//     const char *font_file_path,
//     SDL_Color font_color,
//     int font_size,
//     int *out_width,
//     int *out_height)
// {
//     TTF_Font *font = TTF_OpenFont(font_file_path, font_size);

//     if (!font)
//     {
//         fprintf(stderr, "ERROR: failed to open \"%s\"\n", font_file_path);
//         return 0;
//     }
    
//     SDL_Surface *surface = TTF_RenderText_Blended(font, text, font_color);

//     if (!surface)
//     {
//         TTF_CloseFont(font);
//         fprintf(stderr, "ERROR: failed to render text \"%s\"\n", text);
//         return 0;
//     }

//     GLuint texture_id;
//     glGenTextures(1, &texture_id);
//     glBindTexture(GL_TEXTURE_2D, texture_id);

//     GLenum mode = (surface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
//     glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//     SDL_FreeSurface(surface);
//     TTF_CloseFont(font);

//     if (out_width)
//         *out_width = surface->w;
    
//     if (out_height)
//         *out_height = surface->h;

//     return texture_id;
// }
