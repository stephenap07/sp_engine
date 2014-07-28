#ifndef _SP_FONT_H_
#define _SP_FONT_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include "buffer.h"

#define MAXWIDTH 1024

namespace sp {

struct GlyphInfo
{
    float advance_x;
    float advance_y;

    float bitmap_width;
    float bitmap_height;

    float bitmap_left;
    float bitmap_top;

    float texture_x;
    float texture_y;
};

struct GlyphAtlas
{
    GLuint tex_id;
    Shader shader;
    VertexBuffer buffer;

    int width;
    int height;

    GlyphInfo glyphs[128];

    GlyphAtlas();
    ~GlyphAtlas();

    void LoadFace(FT_Face face, int face_height);
};

void DrawText(const std::string &text_label, GlyphAtlas *atlas, float x, float y, float sx, float sy);

} // namespace sp

#endif
