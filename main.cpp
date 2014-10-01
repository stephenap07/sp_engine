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
#include <glm/gtc/matrix_access.hpp> 

#include <iostream>
#include <vector>
#include <string>
#include <memory>

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
#include "buffer.h"
#include "font.h"
#include "error.h"
#include "system.h"
#include "debug.h"
#include "gui.h"
#include "console.h"
#include "command.h"
#include "model_view.h"

sp::Renderer renderer;

sp::Camera gScreenCamera;

sp::Shader model_program;
sp::Shader plane_program;
sp::Shader skybox_program;
sp::Shader player_program;

sp::TextDefinition *text_def;
sp::Console console;

sp::VertexBuffer cube;
sp::VertexBuffer plane;
sp::VertexBuffer player;

MD5Model md5_model;
sp::IQMModel iqm_model;

GLuint skybox_tex;
GLuint plane_tex;

GLuint skybox_rotate_loc;

float animate = 0.0f;

sp::SystemInfo sys_info;
sp::ModelView p_model(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.5f, 1.0f, 0.5f));
sp::ModelView gun_model(glm::vec3(0.1f, -0.08f, -0.19f), glm::vec3(0.02f, 0.02f, 0.09f));
sp::ModelView iqm_view(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.2));
sp::ModelView block_model(glm::vec3(0.0f, 0.0f, 3.5f), glm::vec3(0.5f, 1.0f, 0.5f));

struct AnimatedObject {
    sp::Shader       *program;
    sp::VertexBuffer *renderable;
    sp::IQMModel     *model;
    sp::ModelView    *view;
};

AnimatedObject mr_fixit;

void InitializeProgram()
{
    model_program.CreateProgram({
        {std::string("assets/shaders/basic_animated.vert"), GL_VERTEX_SHADER},
        {std::string("assets/shaders/gouroud.frag"), GL_FRAGMENT_SHADER}
    });

    plane_program.CreateProgram({
        {std::string("assets/shaders/basic_texture.vs"), GL_VERTEX_SHADER},
        {std::string("assets/shaders/gouroud.frag"), GL_FRAGMENT_SHADER}
    });

    skybox_program.CreateProgram({
        {std::string("assets/shaders/skybox.vert"), GL_VERTEX_SHADER},
        {std::string("assets/shaders/skybox.frag"), GL_FRAGMENT_SHADER}
    });

    player_program.CreateProgram({
        {std::string("assets/shaders/pass_through.vert"), GL_VERTEX_SHADER},
        {std::string("assets/shaders/gouroud.frag"), GL_FRAGMENT_SHADER}
    });

    renderer.LoadGlobalUniforms(model_program.GetID());
    renderer.LoadGlobalUniforms(plane_program.GetID());
    renderer.LoadGlobalUniforms(skybox_program.GetID());
    renderer.LoadGlobalUniforms(player_program.GetID());
}

void Init()
{
    gScreenCamera.Init(
        glm::mat3(
            glm::vec3(0.5f, 0.5f, 2.5f),
            glm::vec3(0.0f, -0.25f, -1.0f),
            glm::vec3(0.0f, 1.0f, 0.0f))
    );

    sys_info.Init();

    InitializeProgram();

    // GUN Init
    //gun_model.scale = glm::vec3(0.03f, 0.03f, 0.14f);
    //gun_model.origin = glm::vec3(0.09f, 0.7f, -0.3f);

    iqm_view.rot = glm::angleAxis(90.0f, glm::vec3(0, 1, 0));

    glm::mat4 model;
    model_program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(model));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    // md5_model.LoadModel("assets/models/hellknight/hellknight.md5mesh");
    // md5_model.LoadAnim("assets/models/hellknight/idle2.md5anim");

    md5_model.LoadModel("assets/models/bob_lamp/boblampclean.md5mesh");
    md5_model.LoadAnim("assets/models/bob_lamp/boblampclean.md5anim");

    //iqm_model.LoadModel("assets/models/imrod_iqm/ImrodLowPoly.iqm");
    //iqm_model.LoadModel("assets/models/hellknight_iqm/hellknight.iqm");
    iqm_model.LoadModel("assets/models/mrfixit/mrfixit.iqm");

    sp::MakeTexturedQuad(&plane);

    sp::MakeCube(&cube, false);

    skybox_tex = sp::MakeTexture("assets/textures/skybox_texture.jpg", GL_TEXTURE_CUBE_MAP);
    glm::mat4 rotate_matrix = glm::scale(glm::mat4(), glm::vec3(300.0f));
    skybox_program.SetUniform(sp::kMatrix4fv, "rotate_matrix", glm::value_ptr(rotate_matrix));

    plane_tex = sp::MakeTexture("assets/textures/checker.tga", GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, plane_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    sp::MakeCube(&player, true);
    glm::vec4 player_color(0.0f, 1.0f, 1.0f, 1.0f);
    //player_program.SetUniform(sp::k4fv, "color_diffuse", glm::value_ptr(player_color));

    model_program.SetUniform(sp::k1i, "is_textured", true);
    model_program.SetUniform(sp::k1i, "is_rigged", true);
    plane_program.SetUniform(sp::k1i, "is_textured", true);

    console.Init(renderer.GetWidth(), renderer.GetHeight());
    sp::font::Init(renderer.GetWidth(), renderer.GetHeight());
    text_def = sp::font::GetTextDef("SPFont3.ttf");

    mr_fixit.program = &model_program;
    //mr_fixit.renderable = 
}

inline void DrawIQM()
{
    model_program.Bind();

    glm::mat4 transform = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 0, 1)
    );
    glm::mat4 model = iqm_view.GetModel() * transform;
    model_program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(model));
    iqm_model.Animate(animate);
    std::vector<glm::mat4> &bones = iqm_model.GetBones();
    model_program.SetUniform(sp::kMatrix4fv, "bone_matrices",
                             bones.size(),
                             glm::value_ptr(bones[0]));
    iqm_model.Render();

    glUseProgram(0);
}

inline void DrawMD5()
{
    model_program.Bind();

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.029f));
    glm::mat4 transform = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 0, -1, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 0, 1)
    );
    model = glm::rotate(model, -55.0f, glm::vec3(0, 1, 0)) * transform;
    model = glm::translate(model, glm::vec3(0.0f, -32.0f, 0.0f));
    model_program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(model));

    glDisable(GL_CULL_FACE);
    md5_model.Render();
    glEnable(GL_CULL_FACE);

    glUseProgram(0);
}

inline void DrawSkyBox()
{
    skybox_program.Bind();

    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);

    glBindVertexArray(cube.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ebo);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glDrawElements(GL_TRIANGLE_FAN, 17, GL_UNSIGNED_SHORT, NULL);
    glDisable(GL_PRIMITIVE_RESTART);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glEnable(GL_CULL_FACE);

    glUseProgram(0);
}

inline void DrawFloor()
{
    plane_program.Bind();

    glm::mat4 plane_model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.0f, 0));
    plane_model = glm::scale(plane_model, glm::vec3(10.0f, 1.0f, 10.0f));
    plane_model = glm::rotate(plane_model, -90.0f, glm::vec3(1, 0, 0));
    plane_program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(plane_model));

    glBindVertexArray(plane.vao);
    glBindBuffer(GL_ARRAY_BUFFER, plane.vbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, plane_tex);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

inline void DrawPlayer()
{
    /** SCALE
     * 1.0 - Player Height
     * 0.5 - Player Width/Depth
     */

    player_program.Bind();
    glm::mat4 player_model = p_model.GetModel();
    player_program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(player_model));

    glBindVertexArray(player.vao);
    glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player.ebo);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glDrawElements(GL_TRIANGLE_STRIP, 17, GL_UNSIGNED_SHORT, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

inline void DrawGun()
{
    player_program.Bind();
    // glm::mat4 world_model = glm::inverse(gScreenCamera.LookAt());
    glm::mat4 g_model = glm::inverse(gScreenCamera.LookAt()) * gun_model.GetModel();
    glm::mat4 gw_model = gScreenCamera.LookAt() * g_model;
    //glm::mat4 gv_model = glm::mat4(1.0f);
    player_program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(g_model));
    player_program.SetUniform(sp::kMatrix4fv, "mv_matrix", glm::value_ptr(gw_model));

    glBindVertexArray(player.vao);
    glBindBuffer(GL_ARRAY_BUFFER, player.vbo);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

inline void DrawBox(float delta)
{
    player_program.Bind();
    block_model.rot = block_model.rot * glm::angleAxis(delta * 180.0f, glm::vec3(0, 1, 0));

    glm::mat4 rot_mat = glm::mat4_cast(block_model.rot);
    glm::mat4 trans_mat = glm::translate(block_model.origin);
    glm::mat4 scale_mat = glm::scale(block_model.scale);

    glm::mat4 model = rot_mat * trans_mat * scale_mat;

    glm::mat4 b_model = model;
    glm::mat4 bv_model = gScreenCamera.LookAt() * model;

    player_program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(b_model));
    player_program.SetUniform(sp::kMatrix4fv, "mv_matrix", glm::value_ptr(bv_model));

    glBindVertexArray(player.vao);
    glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void Display(float delta)
{
    //static float ang = 0.0f;
    renderer.BeginFrame();
    //ang += 30.0f * delta;
    //gScreenCamera.Rotate(2.0f * sin(ang), glm::vec3(0.0f, 0.0f, 1.0f));
    renderer.SetView(gScreenCamera.LookAt());

    //DrawPlayer();
    DrawGun();
    DrawBox(delta);
    DrawFloor();
    //DrawSkyBox();
    DrawMD5();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawIQM();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_DEPTH_TEST);

    text_def->DrawText(std::string("FPS: ") + std::to_string((int)std::ceil((1 / delta))), 8, 35);
    text_def->DrawText(std::string("Platform: ") + sys_info.platform, 8, 50);
    text_def->DrawText(std::string("CPU Count: ") + std::to_string(sys_info.num_cpus), 8, 65);
    text_def->DrawText(std::string("System Ram: ") + std::to_string(sys_info.ram) + std::string("mb"), 8, 80);
    text_def->DrawText(std::string("L1 cache: ") + std::to_string(sys_info.l1_cache) + std::string("kb"), 8, 95);
    text_def->DrawText(std::string("Vendor: ") + (char*)sys_info.vendor, 8, 110);
    text_def->DrawText(std::string("Renderer: ") + (char*)sys_info.renderer, 8, 125);
    text_def->DrawText(std::string("GL Version: ") + (char*)sys_info.version, 8, 140);

    console.Draw();
    glEnable(GL_DEPTH_TEST);
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

    SDL_Event sdl_event;
    const Uint8 *state = nullptr;

    unsigned long elapsed = SDL_GetTicks();
    float delta = 0.0f;

    float player_speed = 10.0f;

    bool hide_mouse = false;
    bool quit = false;
    bool jumping = false;

    const float kGravity = 0.7f;
    float player_vel_y = 0.0f;

    sp::CommandManager::AddCommand("exit", [&quit](const sp::CommandArg &args) { quit = true; });
    sp::CommandManager::AddCommand("fov", [](const sp::CommandArg &args) { renderer.SetAngleOfView(args.GetAs<float>(1)); });

    while (!quit)
    {
        delta = (SDL_GetTicks() - elapsed) / 1000.0f;
        elapsed = SDL_GetTicks();
        animate += 10.0f * delta;

        SDL_StartTextInput();
        while(SDL_PollEvent(&sdl_event)) {
            switch(sdl_event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    Reshape(sdl_event.window.data1, sdl_event.window.data2);
                    break;
            }

            switch(sdl_event.type) {
                case SDL_MOUSEMOTION:
                    gScreenCamera.HandleMouse(sdl_event.motion.xrel,
                                              sdl_event.motion.yrel, delta);
                    p_model.rot = glm::angleAxis(-10.0f *
                                  sdl_event.motion.xrel * delta,
                                  glm::vec3(0.0f, 1.0f, 0.0f)) * p_model.rot;
                    break;
                case SDL_KEYUP:
                    if (sdl_event.key.keysym.sym == SDLK_k) {
                        hide_mouse = !hide_mouse;
                        SDL_SetRelativeMouseMode(hide_mouse ? SDL_TRUE : SDL_FALSE); // hide mouse
                    }
                    if (sdl_event.key.keysym.sym == SDLK_BACKQUOTE) {
                        if (console.FrameIsOpen()) {
                            console.CloseFrame();
                        } else {
                            console.OpenFrame();
                        }
                    }
                    if (sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = true;
                    }
                    break;
                case SDL_QUIT:
                    quit = true;
                    break;
            }

            if (console.FrameIsOpen()) {
                console.HandleEvent(sdl_event);
            }
        }
        SDL_StopTextInput();

        console.Update(delta);

        player_vel_y -= kGravity * delta;
        p_model.origin.y += player_vel_y;

        if (p_model.origin.y < 0.0f) {
            p_model.origin.y = 0.0f;
            player_vel_y = 0.0f;
            jumping = false;
        } else {
            jumping = true;
        }

        // Player Controller
        glm::vec3 side = glm::normalize(glm::cross(gScreenCamera.dir, gScreenCamera.up));
        glm::vec3 forward = glm::normalize(glm::cross(gScreenCamera.up, side));


        if (!console.FrameIsOpen()) {
            state = SDL_GetKeyboardState(nullptr);

            if (state[SDL_SCANCODE_W] && state[SDL_SCANCODE_LGUI]) quit = true;

            if (state[SDL_SCANCODE_W]) p_model.origin += delta * player_speed * forward;
            if (state[SDL_SCANCODE_S]) p_model.origin -= delta * player_speed * forward;

            if (state[SDL_SCANCODE_D]) p_model.origin += delta * player_speed * side;
            if (state[SDL_SCANCODE_A]) p_model.origin -= delta * player_speed * side;

            if (state[SDL_SCANCODE_SPACE] && !jumping) player_vel_y = 0.2f;

            if (state[SDL_SCANCODE_UP]) iqm_view.origin += delta * player_speed * glm::vec3(0, 0, -1);
            if (state[SDL_SCANCODE_DOWN]) iqm_view.origin -= delta * player_speed * glm::vec3(0, 0, -1);

            if (state[SDL_SCANCODE_RIGHT]) iqm_view.origin += delta * player_speed * glm::vec3(1, 0, 0);
            if (state[SDL_SCANCODE_LEFT]) iqm_view.origin -= delta * player_speed * glm::vec3(1, 0, 0);

            //gScreenCamera.FreeRoam(delta);
            gScreenCamera.pos = p_model.origin + glm::vec3(0.0f, 0.8f, 0.0f);
        }

        md5_model.Update(delta);
        Display(delta);
    }

    return EXIT_SUCCESS;
}
