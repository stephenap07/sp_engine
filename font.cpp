#include <GL/glew.h>

#include <iostream>

#include "buffer.h"
#include "shader.h"
#include "font.h"
#include "error.h"

namespace sp {

GlyphAtlas::GlyphAtlas() :width(0), height(0)
{
    buffer.Init();
    /*
    shader.CreateProgram({
        {std::string("assets/shaders/text.vs.glsl"), GL_VERTEX_SHADER},
        {std::string("assets/shaders/text.fs.glsl"), GL_FRAGMENT_SHADER}
    });
    */
}

void GlyphAtlas::LoadFace(FT_Face face, int face_height)
{
    glGetError();

    FT_Set_Pixel_Sizes(face, 0, face_height); 
    FT_GlyphSlot g = face->glyph;

    int row_w = 0;
    int row_h = 0;
    width = height = 0;
    
    memset(glyphs, 0, sizeof(glyphs));

    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed\n", i);
            continue;
        }

        if (row_w + g->bitmap.width + 1 >= MAXWIDTH) {
            width = std::max(width, row_w); 
            height += row_h;
            row_w = row_h = 0;
        }

        row_w += g->bitmap.width + 1;
        row_h = std::max(row_h, g->bitmap.rows);
    }

    width = std::max(width, row_w);
    height += row_h;

    glActiveTexture(GL_TEXTURE0);
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    tex_id = texture_id;

    glBindTexture(GL_TEXTURE_2D, tex_id);
    GLenum e = glGetError();
    if (e) {
        HandleGLError(e);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

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

        if (offset_x + g->bitmap.width + 1 >= MAXWIDTH) {
            offset_y += row_h;
            row_h = 0;
            offset_x = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y,
                        g->bitmap.width, g->bitmap.rows, GL_RED,
                        GL_UNSIGNED_BYTE, (GLubyte*)g->bitmap.buffer);
        glyphs[i] = {
            .advance_x = (float)(g->advance.x >> 6),
            .advance_y = (float)(g->advance.y >> 6),
            .bitmap_width = (float)g->bitmap.width,
            .bitmap_height = (float)g->bitmap.rows,
            .bitmap_left = (float)g->bitmap_left,
            .bitmap_top = (float)g->bitmap_top,
            .texture_x = offset_x / (float)width,
            .texture_y = offset_y / (float)height
        };

        row_h = std::max(row_h, g->bitmap.rows);
        offset_x += g->bitmap.width + 1;
    }

    glBindBuffer(GL_TEXTURE_2D, 0);

    printf("Generated a %d x %d (%d kb) texture atlas\n", width, height, width * height / 1024);
}

GlyphAtlas::~GlyphAtlas()
{
    glDeleteTextures(1, &tex_id);
}

void DrawText(const std::string &text_label, GlyphAtlas *atlas, float x, float y, float sx, float sy)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    atlas->shader.Bind();
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

        coords[c++] = {x2, -y2 - h, 0.0f, glyph.texture_x, glyph.texture_y + s_h};
        coords[c++] = {x2 + w, -y2 - h, 0.0f, glyph.texture_x + s_w, glyph.texture_y + s_h};
        coords[c++] = {x2, -y2, 0.0f, glyph.texture_x, glyph.texture_y};

        coords[c++] = {x2 + w, -y2, 0.0f, glyph.texture_x + s_w, glyph.texture_y};
        coords[c++] = {x2, -y2, 0.0f, glyph.texture_x, glyph.texture_y};
        coords[c++] = {x2 + w, -y2 - h, 0.0f, glyph.texture_x + s_w, glyph.texture_y + s_h};
    }
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, c);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

} // namespace sp
