#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

/* ---------- private structures */

struct text_glyph
{
    GLuint texture;
    int x;
    int y;
    int width;
    int height;
    int pad;
};

/* ---------- private variables */

struct
{
    int glyph_count;
    struct text_glyph *glyphs;
} static text_globals;

/* ---------- public code */

void text_initialize(void)
{
    memset(&text_globals, 0, sizeof(text_globals));

    // Attempt to initialize the freetype library
    FT_Library ft;
    FT_Error result;
    if ((result = FT_Init_FreeType(&ft)) != 0)
    {
        fprintf(stderr, "ERROR: failed to initialize FreeType - %s\n", FT_Error_String(result));
        exit(EXIT_FAILURE);
    }

    // Attempt to load the font face
    FT_Face face;
    if (FT_New_Face(ft, "../assets/fonts/terminus/TerminusTTF-4.49.1.ttf", 0, &face) != 0)
    {
        fprintf(stderr, "ERROR: failed to load font face - %s\n", FT_Error_String(result));
        exit(EXIT_FAILURE);
    }

    // Attempt to set the height of the font
    if (FT_Set_Pixel_Sizes(face, 0, 48) != 0)
    {
        fprintf(stderr, "ERROR: failed to load font glyph - %s\n", FT_Error_String(result));
        exit(EXIT_FAILURE);
    }

    // TODO: create glyph texture atlas
}

void text_dispose(void)
{
    // TODO
}
