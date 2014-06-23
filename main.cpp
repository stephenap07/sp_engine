// Link statically with GLEW
#define GLEW_STATIC

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "shader.h"
#include "md5_model.h"
#include "md5_animation.h"
#include "iqm_model.h"
#include "camera.h"
#include "asset.h"

#define BUFFER_OFFSET(offset) ((void*)(offset))

#define GLSL(src) "#version 330 core\n" #src

const int kScreenWidth = 800;
const int kScreenHeight = 600;

SDL_Window *window = nullptr;
SDL_GLContext context = nullptr;

struct ProgramData {
    GLuint program;
    GLuint uni_block_index;
};

ProgramData model_program;
ProgramData line_program;
ProgramData plane_program;
ProgramData skybox_program;

GLuint global_ubo;

GLuint plane_vbo;
GLuint plane_ebo;
GLuint plane_vao;

GLuint cube_vbo;
GLuint cube_ebo;
GLuint cube_vao;

GLuint skybox_tex;

GLuint skybox_rotate_loc;
GLuint object_mat_mvp_loc;
GLuint object_mat_mv_loc; 

static const GLuint kGlobalUniformBinding = 0;

MD5Model md5_model;
sp::IQMModel iqm_model;

glm::mat4 view;

const std::string kVertexShader("assets/shaders/basic_animated.vert");
const std::string kFragmentShader("assets/shaders/gouroud.frag");
const std::string kLineVertexShader("assets/shaders/line_shader.vert");
const std::string kPassThroughVertexShader("assets/shaders/pass_through.vert");
const std::string kPassThroughFragment("assets/shaders/pass_through.frag");
const std::string kSkyboxVertexShader("assets/shaders/skybox.vert");
const std::string kSkyboxFragmentShader("assets/shaders/skybox.frag");

ProgramData LoadProgram(const std::vector<GLuint> &kShaderList)
{
    ProgramData data;
    data.program = sp::shader::CreateProgram(kShaderList);
    data.uni_block_index = glGetUniformBlockIndex(data.program, "globalMatrices");
    glUniformBlockBinding(data.program, data.uni_block_index, kGlobalUniformBinding);

    return data;
}

void InitializeProgram()
{
    std::vector<GLuint> shader_list = {
        sp::shader::CreateShader(GL_VERTEX_SHADER, kVertexShader),
        sp::shader::CreateShader(GL_FRAGMENT_SHADER, kFragmentShader)
    };
    std::vector<GLuint> line_shader_list = {
        sp::shader::CreateShader(GL_VERTEX_SHADER, kLineVertexShader),
        sp::shader::CreateShader(GL_FRAGMENT_SHADER, kPassThroughFragment)
    };
    std::vector<GLuint> plane_shader_list = {
        sp::shader::CreateShader(GL_VERTEX_SHADER, kPassThroughVertexShader),
        sp::shader::CreateShader(GL_FRAGMENT_SHADER, kPassThroughFragment)
    };
    std::vector<GLuint> skybox_shader_list = {
        sp::shader::CreateShader(GL_VERTEX_SHADER, kSkyboxVertexShader),
        sp::shader::CreateShader(GL_FRAGMENT_SHADER, kSkyboxFragmentShader)
    };

    model_program = LoadProgram(shader_list);
    line_program = LoadProgram(line_shader_list);
    plane_program = LoadProgram(plane_shader_list);
    skybox_program = LoadProgram(skybox_shader_list);

    std::for_each(shader_list.begin(), shader_list.end(), glDeleteShader);
}

void Init()
{
    glm::mat4 model;
    sp::InitCamera();
    view = sp::CameraLookAt();

    glm::mat4 proj = glm::perspective(60.0f, (float)kScreenWidth / kScreenHeight, 0.01f, 1024.0f);

    glGenBuffers(1, &global_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER,
            kGlobalUniformBinding,
            global_ubo, 0, 2 * sizeof(glm::mat4));

    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj)); 
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

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

    md5_model.LoadModel("assets/models/bob_lamp/boblampclean.md5mesh");
    md5_model.LoadAnim("assets/models/bob_lamp/boblampclean.md5anim");
    iqm_model.LoadModel("assets/models/mrfixit/mrfixit.iqm");

    GLfloat vert_plane[] = {
        -1.0f,  1.0f, -1.0f, // Top Left
        1.0f,  1.0f, -1.0f,  // Top Right
        1.0f, -1.0f, 1.0f,   // Bottom Right
        -1.0f, -1.0f, 1.0f   // Bottom Left
    };

    GLuint vert_indices[] = {
        3, 2, 1,
        1, 0, 3
    };

    glGenBuffers(1, &plane_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vert_indices), vert_indices,
            GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &plane_vao);
    glBindVertexArray(plane_vao);
    glGenBuffers(1, &plane_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, plane_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert_plane), vert_plane, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Cube map
    
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

    static const GLfloat cube_vertices[] =
    {
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f
    };

    static const GLushort cube_indices[] =
    {
        0, 1, 2, 3, 6, 7, 4, 5,         // First strip
        2, 6, 0, 4, 1, 5, 3, 7          // Second strip
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &cube_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    skybox_rotate_loc = glGetUniformLocation(skybox_program.program, "tc_rotate");
    skybox_tex = sp::MakeTexture("assets/textures/skybox_texture.jpg", GL_TEXTURE_CUBE_MAP);
}

void FreeResources()
{
    glDeleteProgram(model_program.program);
    glDeleteProgram(line_program.program);
    glDeleteProgram(plane_program.program);
    glDeleteProgram(skybox_program.program);

    glDeleteVertexArrays(1, &plane_vao);
    glDeleteBuffers(1, &plane_vbo);
    glDeleteBuffers(1, &global_ubo);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void Display()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view = sp::CameraLookAt();

    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER,
                    sizeof(glm::mat4),
                    sizeof(glm::mat4),
                    glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Begin Cube Map Drawing
    glDisable(GL_CULL_FACE);
    glUseProgram(skybox_program.program);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);
    
    glm::mat4 proj = glm::perspective(60.0f, (float)kScreenWidth / kScreenHeight, 0.01f, 1024.0f);
    glm::mat4 tc_matrix;
    tc_matrix = glm::scale(proj * view * tc_matrix, glm::vec3(300.0f));
    glUniformMatrix4fv(skybox_rotate_loc, 1, GL_FALSE, glm::value_ptr(tc_matrix));

    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

    glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, NULL);
    glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, BUFFER_OFFSET(8 * sizeof(GLushort)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glEnable(GL_CULL_FACE);
    // End Cube Drawing

    glUseProgram(model_program.program);

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
    glm::mat4 transform = glm::mat4(
            glm::vec4(1, 0, 0, 0),
            glm::vec4(0, 0, -1, 0),
            glm::vec4(0, 1, 0, 0),
            glm::vec4(0, 0, 0, 1)
            );
    model = glm::rotate(model, -55.0f, glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(0.0f, -50.0f, 0.0f));

    model *= transform;

    GLint uni_model_matrix = glGetUniformLocation(model_program.program,
                                                  "model_matrix");
    glUniformMatrix4fv(uni_model_matrix, 1, GL_FALSE, glm::value_ptr(model)); 

    glDisable(GL_CULL_FACE);
    md5_model.Render();
    glEnable(GL_CULL_FACE);

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

    iqm_model.AnimateIQM(10.0f * SDL_GetTicks() / 1000.0f);
    iqm_model.Render();

    glFrontFace(GL_CCW);

    glUseProgram(line_program.program);
    glUniformMatrix4fv(uni_model_matrix, 1, GL_FALSE, glm::value_ptr(model)); 

    // md5_model.RenderNormals();

    glDisable(GL_CULL_FACE);
    glUseProgram(plane_program.program);

    glm::mat4 plane_model;
    plane_model = glm::translate(plane_model, glm::vec3(0, -1.0f, 0));
    plane_model = glm::scale(plane_model, glm::vec3(10.0f, 1.0f, 10.0f));
    plane_model = glm::rotate(plane_model, -45.0f, glm::vec3(1, 0, 0));
    GLint uni_plane_model_matrix = glGetUniformLocation(plane_program.program, "model_matrix");
    glUniformMatrix4fv(uni_plane_model_matrix, 1, GL_FALSE, glm::value_ptr(plane_model)); 

    glBindVertexArray(plane_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);

    SDL_GL_SwapWindow(window);
}

void Reshape (int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);

    window = SDL_CreateWindow(
        "MD5 model Viewer", 100, 100,
        kScreenWidth, kScreenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    context = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    glewInit();

    SDL_Event window_ev;
    const Uint8 *state = nullptr;

    Init();
    SDL_SetRelativeMouseMode(SDL_TRUE);

    unsigned long elapsed = SDL_GetTicks();
    float delta = 0.0f;

    bool quit = false;
    while (!quit)
    {
        delta = (SDL_GetTicks() - elapsed) / 1000.0f;
        elapsed = SDL_GetTicks();

        state = SDL_GetKeyboardState(nullptr);
        if (state[SDL_SCANCODE_W] && state[SDL_SCANCODE_LGUI]) quit = true;
        if (state[SDL_SCANCODE_ESCAPE]) quit = true;
        if (state[SDL_SCANCODE_W]) {
            sp::MoveCameraForward(delta);
        }
        if (state[SDL_SCANCODE_S]) {
            sp::MoveCameraBackward(delta);
        }
        if (state[SDL_SCANCODE_D]) {
            sp::MoveCameraRight(delta);
        }
        if (state[SDL_SCANCODE_A]) {
            sp::MoveCameraLeft(delta);
        } 

        while(SDL_PollEvent(&window_ev)) {
            switch(window_ev.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    Reshape(window_ev.window.data1, window_ev.window.data2);
                    break;
            }

            switch(window_ev.type) {
                case SDL_MOUSEMOTION:
                    sp::HandleMouse(window_ev.motion.xrel, window_ev.motion.yrel, delta);
                    break;
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }

        md5_model.Update(delta);
        Display();
    }


    FreeResources();

    return EXIT_SUCCESS;
}
