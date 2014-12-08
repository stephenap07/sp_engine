#ifndef _SP_FONT_H_
#define _SP_FONT_H_

#include <string>
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

class GlyphAtlas
{
public:
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

class TextDefinition
{
public:
    bool Init(const char *font_name, float window_width, float window_height);
    bool IsInit () const { return is_initialized; }
    void DrawText(const std::string &label, float x, float y);

private:
    // TODO: Integrate with vertex cache
    sp::VertexBuffer text_buffer;
    sp::Shader text_program;
    GlyphAtlas atlas_48;
    GlyphAtlas atlas_24;
    GlyphAtlas atlas_16;

    FT_Library ft;
    FT_Face face;
    bool is_initialized;

    // TODO: Do proper scaling globally?
    // Should be a more elegant way to deal with scaling images in the window
    float window_width, window_height;
};

void DrawText(const std::string &text_label, GlyphAtlas *atlas, float x, float y, float sx, float sy);

namespace font {
    bool Init(float window_width, float window_height);
    TextDefinition *const GetTextDef(const std::string &text_def_name);    
}

} // namespace sp

#endif
