/*
 * Copyright (C) 2014 Stephen Pridham
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "md5_model.h"
#include "md5_animation.h"
#include "iqm_model.h"
#include "camera.h"
#include "asset.h"
#include "geometry.h"
#include "util.h"
#include "logger.h"
#include "renderer.h"
#include "shader.h"

sp::Renderer renderer;

sp::Camera gScreenCamera;

sp::ProgramData model_program;
sp::ProgramData line_program;
sp::ProgramData plane_program;
sp::ProgramData skybox_program;
sp::ProgramData text_program;

sp::Renderable cube;
sp::Renderable plane;
sp::Renderable text;

MD5Model md5_model;
sp::IQMModel iqm_model;

GLuint skybox_tex;
GLuint plane_tex;
GLuint text_tex;

int text_w, text_h;

GLuint skybox_rotate_loc;
GLuint object_mat_mvp_loc;
GLuint object_mat_mv_loc; 

float animate = 0.0f;

TTF_Font *font = nullptr;

#define MAXWIDTH 1024

GLuint uniform_color;

FT_Library ft;
FT_Face face;

struct TexturePoint
{
    GLfloat x, y, z, s, t;
};

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

    int width;
    int height;

    GlyphInfo glyphs[128];

    GlyphAtlas(FT_Face face, int height)
    {
        FT_Set_Pixel_Sizes(face, 0, height); 
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
        glGenTextures(1, &tex_id);
        glBindTexture(tex_id, GL_TEXTURE_2D);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);

        glPixelStorei(GL_UNPACK_ALIGNMENT, GL_TRUE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int offset_x = 0;
        int offset_y = 0;
        row_h = 0;

        for (int i = 32; i < 128; i++) {
            if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
                sp::log::ErrorLog("Loading character %c failed\n", i);
                continue;
            }

            if (offset_x + g->bitmap.width + 1 >= MAXWIDTH) {
                offset_y += row_h;
                row_h = 0;
                offset_x = 0;
            }

            glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y,
                            g->bitmap.width, g->bitmap.rows, GL_ALPHA,
                            GL_UNSIGNED_BYTE, g->bitmap.buffer);
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
        sp::log::InfoLog("Generated a %d x %d (%d kb) texture atlas\n", width, height, width * height / 1024);
    }

    ~GlyphAtlas()
    {
        glDeleteTextures(1, &tex_id);
    }
};

std::unique_ptr<GlyphAtlas> g_atlas_48;
std::unique_ptr<GlyphAtlas> g_atlas_24;
std::unique_ptr<GlyphAtlas> g_atlas_16;

std::ostream& operator<<(std::ostream& os, const sp::Camera& cam)
{
    auto print_vec3 = [&os](glm::vec3 a) {os << a[0] << ", " << a[1] << ", " << a[2];};
    os << "camera pos: "; print_vec3(cam.pos); os << std::endl;
    os << "camera dir: "; print_vec3(cam.dir); os << std::endl;
    os << "camera up: "; print_vec3(cam.up); os << std::endl;
    os << "camera look: "; print_vec3(cam.look);
    return os;
}

void InitializeProgram()
{
    model_program  = renderer.LoadProgram({
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/basic_animated.vert"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER, "assets/shaders/gouroud.frag"})
    });
    line_program   = renderer.LoadProgram({
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/line_shader.vert"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER, "assets/shaders/pass_through.frag"})
    });
    plane_program  = renderer.LoadProgram({
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/basic_texture.vs"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER,  "assets/shaders/gouroud.frag"})
    });
    skybox_program = renderer.LoadProgram({
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/skybox.vert"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER, "assets/shaders/skybox.frag"})
    });
    text_program = renderer.LoadProgram({
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/2d.vert"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER, "assets/shaders/2d.frag"}),
    });
}

inline int ClosestPowerOfTwo(int num)
{
    int result = 2;
    while (result < num) {
        result *= 2;
    }
    return result;
}

bool InitializeFontMap()
{
    sp::MakeTexturedQuad(&text);
    
    if (FT_Init_FreeType(&ft)) {
        sp::log::ErrorLog("Could not init freetype library\n");
        return false;
    }
    if (FT_New_Face(ft, "assets/fonts/FreeMono.ttf", 0, &face)) {
        sp::log::ErrorLog("Could not open font %s\n", "FreeSans.ttf");
    }

    g_atlas_48.reset(new GlyphAtlas(face, 48));
    g_atlas_24.reset(new GlyphAtlas(face, 24));
    g_atlas_16.reset(new GlyphAtlas(face, 16));

    glUseProgram(text_program.program);
    uniform_color = glGetUniformLocation(text_program.program, "uni_color");

    GLfloat green[4] = {0, 1, 0, 1};
    glUniform4fv(uniform_color, 1, green);
    
    return true;
}

void Init()
{
    gScreenCamera.Init(
        glm::mat3(
            glm::vec3(0.5f, 0.5f, 2.5f),
            glm::vec3(0.0f, -0.25f, -1.0f),
            glm::vec3(0.0f, 1.0f, 0.0f))
    );

    glm::mat4 model;

    InitializeProgram();

    InitializeFontMap(); 

    glUseProgram(model_program.program);

    GLint uni_model_matrix = glGetUniformLocation(model_program.program, "model_matrix");
    glUniformMatrix4fv(uni_model_matrix, 1, GL_FALSE, glm::value_ptr(model)); 

    glUseProgram(line_program.program);

    uni_model_matrix = glGetUniformLocation(line_program.program, "model_matrix");
    glUniformMatrix4fv(uni_model_matrix, 1, GL_FALSE, glm::value_ptr(model)); 

    glUseProgram(0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    // md5_model.LoadModel("assets/models/hellknight/hellknight.md5mesh");
    // md5_model.LoadAnim("assets/models/hellknight/idle2.md5anim");

    // md5_model.LoadModel("assets/models/bob_lamp/boblampclean.md5mesh");
    // md5_model.LoadAnim("assets/models/bob_lamp/boblampclean.md5anim");

    iqm_model.LoadModel("assets/models/mrfixit/mrfixit.iqm");

    sp::MakeTexturedQuad(&plane);
    sp::MakeCube(&cube);

    skybox_rotate_loc = glGetUniformLocation(skybox_program.program, "tc_rotate");
    skybox_tex = sp::MakeTexture("assets/textures/skybox_texture.jpg", GL_TEXTURE_CUBE_MAP);
    plane_tex = sp::MakeTexture("assets/textures/checker.tga", GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, plane_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

inline void DrawIQM()
{
    glUseProgram(model_program.program);
    GLint uni_model_matrix = glGetUniformLocation(model_program.program, "model_matrix");
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
    glm::mat4 transform = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 0, -1, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 0, 1)
    );
    model = glm::rotate(model, -55.0f, glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(0.0f, -50.0f, 0.0f)) * transform;
    model = glm::translate(model, glm::vec3(30.0f, -1.5f, 0.0f));
    model = glm::scale(model, glm::vec3(7.0f));

    glUniformMatrix4fv(uni_model_matrix, 1, GL_FALSE, glm::value_ptr(model)); 
    GLint uni_bone_matrices = glGetUniformLocation(model_program.program,
                                                   "bone_matrices");
                                                    
    glUniformMatrix4fv(uni_bone_matrices,
                       iqm_model.out_frames.size(),
                       GL_FALSE,
                       glm::value_ptr(iqm_model.out_frames[0])); 

    glFrontFace(GL_CW);
    iqm_model.AnimateIQM(animate);
    iqm_model.Render();
    glFrontFace(GL_CCW);

    glUseProgram(0);
}

inline void DrawMD5()
{
    glUseProgram(model_program.program);

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
    glm::mat4 transform = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 0, -1, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 0, 1)
    );
    model = glm::rotate(model, -55.0f, glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(0.0f, -50.0f, 0.0f)) * transform;
    GLint uni_model_matrix = glGetUniformLocation(model_program.program,
                                                  "model_matrix");
    glUniformMatrix4fv(uni_model_matrix, 1, GL_FALSE, glm::value_ptr(model)); 

    glDisable(GL_CULL_FACE);
    md5_model.Render();
    glEnable(GL_CULL_FACE);

    glUseProgram(0);
}

inline void DrawSkyBox()
{
    glDisable(GL_CULL_FACE);
    glUseProgram(skybox_program.program);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);

    glm::mat4 tc_matrix = glm::scale(glm::mat4(), glm::vec3(300.0f));
    glUniformMatrix4fv(skybox_rotate_loc, 1, GL_FALSE, glm::value_ptr(tc_matrix));

    glBindVertexArray(cube.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ebo);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);

    glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, NULL);
    glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, BUFFER_OFFSET(8 * sizeof(GLushort)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glEnable(GL_CULL_FACE);
}

inline void DrawFloor()
{
    glDisable(GL_CULL_FACE);
    glUseProgram(plane_program.program);

    glm::mat4 plane_model;
    plane_model = glm::translate(plane_model, glm::vec3(0, -1.0f, 0));
    plane_model = glm::scale(plane_model, glm::vec3(10.0f, 1.0f, 10.0f));
    plane_model = glm::rotate(plane_model, -90.0f, glm::vec3(1, 0, 0));
    GLint uni_plane_model_matrix = glGetUniformLocation(plane_program.program, "model_matrix");
    glUniformMatrix4fv(uni_plane_model_matrix, 1, GL_FALSE, glm::value_ptr(plane_model)); 

    glBindVertexArray(plane.vao);
    glBindBuffer(GL_ARRAY_BUFFER, plane.vbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, plane_tex);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glEnable(GL_CULL_FACE);
}

void DrawText(const std::string &text_label, const std::unique_ptr<GlyphAtlas> &atlas, float x, float y, float sx, float sy)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(text_program.program);

    glBindTexture(GL_TEXTURE_2D, atlas->tex_id);

    glBindVertexArray(text.vao);
    glBindBuffer(GL_ARRAY_BUFFER, text.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text.ebo);

    TexturePoint coords[4 * text_label.size()];
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

        coords[c++] = {x2, -y2, 0.0f, glyph.texture_x, glyph.texture_y};
        coords[c++] = {x2 + w, -y2, 0.0f, glyph.texture_x + glyph.bitmap_width / atlas->width, glyph.texture_y};
        coords[c++] = {x2 + w, -y2 - h, 0.0f, glyph.texture_x + glyph.bitmap_width / atlas->width, glyph.texture_y + glyph.bitmap_height / atlas->height};
        coords[c++] = {x2, -y2 - h, 0.0f, glyph.texture_x, glyph.texture_y + glyph.bitmap_height / atlas->height};
    }
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, c);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void Display()
{
    float sx = 2.0f / renderer.GetWidth();
    float sy = 2.0f / renderer.GetHeight();

    renderer.BeginFrame();
    renderer.SetView(gScreenCamera.LookAt());

    DrawSkyBox();
    DrawFloor();

    DrawIQM();
    DrawMD5();

    DrawText("The Quick Brown Fox Jumps Over The Lazy Dog", g_atlas_24, -1 + 8 * sx, 1 - 50 * sy, sx, sy);
    // DrawText("The Quick Brown Fox Jumps Over The Lazy Dog", g_atlas_16, -1 + 9 * sx, 1 - 50 * sy, sx, sy);
    //DrawText("The Small Texture Scaled Fox Jumps Over The Lazy Dog", g_atlas_48, -1.0f + 8.0f * sx, 1.0f - 175.0f * sy, sx * 0.5f, sy);

    renderer.EndFrame();
}

void Reshape (int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

int main()
{
    renderer.Init();
    Init();

    SDL_Event window_ev;
    const Uint8 *state = nullptr;

    unsigned long elapsed = SDL_GetTicks();
    float delta = 0.0f;

    bool hide_mouse = false;
    bool quit = false;
    while (!quit)
    {
        delta = (SDL_GetTicks() - elapsed) / 1000.0f;
        elapsed = SDL_GetTicks();
        animate += 10.0f * delta;

        state = SDL_GetKeyboardState(nullptr);
        if (state[SDL_SCANCODE_W] && state[SDL_SCANCODE_LGUI]) quit = true;
        if (state[SDL_SCANCODE_ESCAPE]) quit = true;

        gScreenCamera.FreeRoam(delta);

        while(SDL_PollEvent(&window_ev)) {
            switch(window_ev.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    Reshape(window_ev.window.data1, window_ev.window.data2);
                    break;
            }

            switch(window_ev.type) {
                case SDL_MOUSEMOTION:
                    gScreenCamera.HandleMouse(window_ev.motion.xrel, window_ev.motion.yrel, delta);
                    break;
                case SDL_KEYUP:
                    if (window_ev.key.keysym.sym == SDLK_k) {
                        // std::cout << gScreenCamera << std::endl;
                        hide_mouse = !hide_mouse;
                        SDL_SetRelativeMouseMode(hide_mouse ? SDL_TRUE : SDL_FALSE); // hide mouse
                    }
                    break;
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }

        //md5_model.Update(delta);
        Display();
    }

    return EXIT_SUCCESS;
}
