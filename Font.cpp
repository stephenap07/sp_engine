#include <GL/glew.h>

#include <iostream>
#include <algorithm>

#include "VertexBuffer.hpp"
#include "Shader.hpp"
#include "Font.hpp"
#include "Error.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace sp
{

GlyphAtlas::GlyphAtlas() : width(0), height(0) { buffer.Init(); }

void GlyphAtlas::LoadFace(FT_Face face, int face_height)
{
    glGetError(); // clear errors, TODO: Remove this
    FT_Set_Pixel_Sizes(face, 0, face_height);
    FT_GlyphSlot glyphSlot = face->glyph;

    int row_w = 0;
    int row_h = 0;
    width = height = 0;

    memset(glyphs, 0, sizeof(glyphs));

    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT)) {
            fprintf(stderr, "Loading character %c failed\n", i);
            continue;
        }

        if (row_w + glyphSlot->bitmap.width + 1 >= MAXWIDTH) {
            width = std::max(width, row_w);
            height += row_h;
            row_w = row_h = 0;
        }

        row_w += glyphSlot->bitmap.width + 1;
        row_h = std::max(row_h, static_cast<int>(glyphSlot->bitmap.rows));
    }

    width = std::max(width, row_w);
    height += row_h;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex_id);

    glBindTexture(GL_TEXTURE_2D, tex_id);
    HandleGLError(glGetError());

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int offset_x = 0;
    int offset_y = 0;
    row_h = 0;

    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            std::cerr << "Loading character " << (char)i << " failed\n";
            continue;
        }

        if (offset_x + glyphSlot->bitmap.width + 1 >= MAXWIDTH) {
            offset_y += row_h;
            row_h = 0;
            offset_x = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y,
                        glyphSlot->bitmap.width, glyphSlot->bitmap.rows, GL_RED,
                        GL_UNSIGNED_BYTE, (GLubyte *)glyphSlot->bitmap.buffer);
        glyphs[i] = {(float)(glyphSlot->advance.x >> 6),
                     (float)(glyphSlot->advance.y >> 6),
                     (float)glyphSlot->bitmap.width,
                     (float)glyphSlot->bitmap.rows,
                     (float)glyphSlot->bitmap_left,
                     (float)glyphSlot->bitmap_top,
                     offset_x / (float)width,
                     offset_y / (float)height};

        row_h = std::max(row_h, static_cast<int>(glyphSlot->bitmap.rows));
        offset_x += glyphSlot->bitmap.width + 1;
    }
}

GlyphAtlas::~GlyphAtlas() { glDeleteTextures(1, &tex_id); }

void DrawText(const std::string &text_label, GlyphAtlas *atlas, float x,
              float y, float sx, float sy)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    sp::backend::Bind(atlas->shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas->tex_id);

    glBindVertexArray(atlas->buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, atlas->buffer.vbo);

    Point coords[6 * text_label.size()];
    int c = 0;

    for (auto ch : text_label) {
        GlyphInfo glyph = atlas->glyphs[(int)ch];

        float x2 = x + glyph.bitmap_left * sx;
        float y2 = -y - glyph.bitmap_top * sy;
        float w = glyph.bitmap_width * sx;
        float h = glyph.bitmap_height * sy;

        x += glyph.advance_x * sx;
        y += glyph.advance_y * sy;

        if (!w || !h) {
            continue;
        }

        float s_w = glyph.bitmap_width / atlas->width;
        float s_h = glyph.bitmap_height / atlas->height;

        coords[c++] = {x2, -y2 - h, 0.0f, glyph.texture_x,
                       glyph.texture_y + s_h};
        coords[c++] = {x2 + w, -y2 - h, 0.0f, glyph.texture_x + s_w,
                       glyph.texture_y + s_h};
        coords[c++] = {x2, -y2, 0.0f, glyph.texture_x, glyph.texture_y};

        coords[c++] = {x2 + w, -y2, 0.0f, glyph.texture_x + s_w,
                       glyph.texture_y};
        coords[c++] = {x2, -y2, 0.0f, glyph.texture_x, glyph.texture_y};
        coords[c++] = {x2 + w, -y2 - h, 0.0f, glyph.texture_x + s_w,
                       glyph.texture_y + s_h};
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, c);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

//==============================================================================

bool TextDefinition::Init(float width, float height)
{
    window_width = 2.0f / width;
    window_height = 2.0f / height;

    text_program = backend::CreateProgram(
        {{"assets/shaders/text.vs.glsl", GL_VERTEX_SHADER},
         {"assets/shaders/text.fs.glsl", GL_FRAGMENT_SHADER}});

    text_buffer = sp::MakeTexturedQuad(GL_DYNAMIC_DRAW);

    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Could not init freetype library\n";
        return false;
    }

    if (FT_New_Face(ft, "assets/fonts/FreeMonoBold.ttf", 0, &face)) {
        std::cerr << "Could not open font\n";
        return false;
    }

    glm::mat4 model;
    backend::SetUniform(text_program, sp::kMatrix4fv, "uni_model",
                        glm::value_ptr(model));
    glm::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
    backend::SetUniform(text_program, sp::k4fv, "uni_color",
                        glm::value_ptr(white));

    atlas_48.LoadFace(face, 48);
    atlas_24.LoadFace(face, 24);
    atlas_16.LoadFace(face, 16);

    // TODO: Automatically set program & buffer via vertex/program cache
    atlas_48.shader = text_program;
    atlas_24.shader = text_program;
    atlas_16.shader = text_program;

    atlas_48.buffer = text_buffer;
    atlas_24.buffer = text_buffer;
    atlas_16.buffer = text_buffer;

    return true;
}

//==============================================================================

void TextDefinition::DrawText(const std::string &label, float x, float y)
{
    ::sp::DrawText(label, &atlas_16, -1 + x * window_width,
                   1 - y * window_height, window_width, window_height);
}

//==============================================================================

namespace font
{
static TextDefinition gTextDef;

bool Init(float window_width, float window_height)
{
    return gTextDef.Init(window_width, window_height);
}

TextDefinition *const GetTextDef(const std::string &text_def_name)
{
    return &gTextDef;
}
}

} // namespace sp
