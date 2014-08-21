#include <string>
#include <iostream>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 

#include "geometry.h"
#include "gui.h"

GUIFrame::GUIFrame(float width, float height)
{
    this->width = width; 
    this->height = height;
}

GUIFrame::~GUIFrame()
{
    std::cout << "Called\n";
}

void GUIFrame::Init()
{
    program.CreateProgram({
        {std::string("assets/shaders/2d.vert"), GL_VERTEX_SHADER},
        {std::string("assets/shaders/2d.frag"), GL_FRAGMENT_SHADER},
    });

    // TODO: Find nice way to orientate user interface
    sp::MakeQuad(&buffer);
    glm::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);

    float scale_x = 2.0f / width;
    float scale_y = 2.0f / height;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 0));
    model = glm::scale(model, glm::vec3(scale_x, scale_y, 0));
    model = glm::translate(model, glm::vec3(400, -400, 0));
    model = glm::scale(model, glm::vec3(100, 200, 0));

    program.SetUniform(sp::k4fv, "uni_color", glm::value_ptr(white));
    program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(model));
}

void GUIFrame::Draw()
{
    program.Bind();
    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ebo);

    // NOTE: Take note of types GL_UNSIGNED_SHORT
    // Make sure it's the same in the data buffer
    // Made a mistake when it was GLuint instead of GLushort!
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
