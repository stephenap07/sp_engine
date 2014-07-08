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

// Link statically with GLEW
#define GLEW_STATIC

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
#include <algorithm>

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

GLuint global_ubo;

sp::Renderable cube;
sp::Renderable plane;

GLuint skybox_tex;
GLuint plane_tex;

GLuint skybox_rotate_loc;
GLuint object_mat_mvp_loc;
GLuint object_mat_mv_loc; 

MD5Model md5_model;
sp::IQMModel iqm_model;

float animate = 0.0f;

void InitializeProgram()
{
    std::vector<GLuint> shader_list = {
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/basic_animated.vert"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER, "assets/shaders/gouroud.frag"})
    };
    std::vector<GLuint> line_shader_list = {
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/line_shader.vert"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER, "assets/shaders/pass_through.frag"})
    };
    std::vector<GLuint> plane_shader_list = {
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/basic_texture.vs"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER,  "assets/shaders/gouroud.frag"})
    };
    std::vector<GLuint> skybox_shader_list = {
        sp::shader::CreateShader({GL_VERTEX_SHADER, "assets/shaders/skybox.vert"}),
        sp::shader::CreateShader({GL_FRAGMENT_SHADER, "assets/shaders/skybox.frag"})
    };

    model_program = renderer.LoadProgram(shader_list);
    line_program = renderer.LoadProgram(line_shader_list);
    plane_program = renderer.LoadProgram(plane_shader_list);
    skybox_program = renderer.LoadProgram(skybox_shader_list);

    std::for_each(shader_list.begin(), shader_list.end(), glDeleteShader);
}

void Init()
{
    glm::mat4 model;

    InitializeProgram();
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

void FreeResources()
{
    glDeleteProgram(model_program.program);
    glDeleteProgram(line_program.program);
    glDeleteProgram(plane_program.program);
    glDeleteProgram(skybox_program.program);

}

void Display()
{
    renderer.SetView(gScreenCamera.LookAt());
    renderer.BeginFrame();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Begin cube drawing
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
    // End cube drawing

    // Begin md5 model drawing
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
    //md5_model.Render();
    glEnable(GL_CULL_FACE);
    // End md5 model drawing

    // Begin iqm model drawing
    model = glm::translate(model, glm::vec3(30.0f, -1.5f, 0.0f));
    model = glm::scale(model, glm::vec3(7.0f));
    glUniformMatrix4fv(uni_model_matrix, 1, GL_FALSE, glm::value_ptr(model)); 
    GLint uni_bone_matrices = glGetUniformLocation(model_program.program,
                                                   "bone_matrices");
                                                    
    iqm_model.AnimateIQM(animate);
    glUniformMatrix4fv(uni_bone_matrices,
                       iqm_model.out_frames.size(),
                       GL_FALSE,
                       glm::value_ptr(iqm_model.out_frames[0])); 

    glFrontFace(GL_CW);
    iqm_model.AnimateIQM(animate);
    iqm_model.Render();
    glFrontFace(GL_CCW);
    // End iqm model drawing

    // Begin normals drawing
    glUseProgram(line_program.program);
    glUniformMatrix4fv(uni_model_matrix, 1, GL_FALSE, glm::value_ptr(model)); 

    // md5_model.RenderNormals();

    // End normals drawing

    // Begin plane drawing
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
    // End plane drawing

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
    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_Event window_ev;
    const Uint8 *state = nullptr;

    unsigned long elapsed = SDL_GetTicks();
    float delta = 0.0f;

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
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }

        //md5_model.Update(delta);
        Display();
    }

    FreeResources();

    return EXIT_SUCCESS;
}
