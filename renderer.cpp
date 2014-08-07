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

#include <vector>
#include <string>
#include <algorithm>

#include "renderer.h"
#include "shader.h"
#include "error.h"
#include "logger.h"

namespace sp {

//------------------------------------------------------------------------------

void Renderer::Init()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);

    screen_width = 1280;
    screen_height = 720;

    window = SDL_CreateWindow(
        "SP Engine Test", 100, 100,
        screen_width, screen_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        log::ErrorLog("SDL_GL_CreateContext: FATAL - "
                      "Failed to created GL context\n");
    }

    glewExperimental = GL_TRUE;
    GLenum glew_error = glewInit();

    if (glew_error != GLEW_OK) {
        log::ErrorLog("glewInit failed, aborting\n");
    }

    GLenum error_enum = glGetError();
    if (error_enum != GL_INVALID_ENUM) {
        HandleGLError(error_enum);
    }

    projection = glm::perspective(60.0f, (float)screen_width / screen_height, 0.01f, 1024.0f);

    glGenBuffers(1, &global_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, global_uniform_binding,
                      global_ubo, 0, 2 * sizeof(glm::mat4));

    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection)); 
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//------------------------------------------------------------------------------

void Renderer::BeginFrame()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_UNIFORM_BUFFER, global_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER,
                    sizeof(glm::mat4),
                    sizeof(glm::mat4),
                    glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//------------------------------------------------------------------------------

void Renderer::EndFrame()
{
    SDL_GL_SwapWindow(window);
}

//------------------------------------------------------------------------------

void Renderer::FreeResources()
{
    for(auto it = program_store.begin(); it != program_store.end(); it++) {
        glDeleteProgram(it->program);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

//------------------------------------------------------------------------------

Renderer::~Renderer()
{
    FreeResources();
}

//------------------------------------------------------------------------------

void Renderer::SetView(const glm::mat4 &view)
{
   this->view = view; 
}

//------------------------------------------------------------------------------

void Renderer::LoadGlobalUniforms(GLuint shader_index)
{
    GLint uni_block_index = glGetUniformBlockIndex(shader_index, "globalMatrices");
    glUniformBlockBinding(shader_index, uni_block_index, global_uniform_binding);
}

} // namespace sp
