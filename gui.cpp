#include <string>
#include <iostream>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 

#include "geometry.h"
#include "gui.h"

GUIFrame::GUIFrame(float x, float y, float sx, float sy, float width, float height)
{
    Init(x, y, sx, sy, width, height);
}

GUIFrame::~GUIFrame()
{
}

void GUIFrame::Init(float x, float y, float sx, float sy, float width, float height)
{
    this->x = x;
    this->y = y;
    this->scale_x = sx;
    this->scale_y = sy;

    program.CreateProgram({
        {std::string("assets/shaders/2d.vert"), GL_VERTEX_SHADER},
        {std::string("assets/shaders/2d.frag"), GL_FRAGMENT_SHADER},
    });

    sp::MakeQuad(&buffer);

    SetColor(glm::vec4(0.3f, 0.4f, 0.5f, 0.8f));
    SetSize(width, height);
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

void GUIFrame::SetColor(const glm::vec4 &new_color)
{
    color = new_color;
    program.SetUniform(sp::k4fv, "uni_color", glm::value_ptr(color));
}

void GUIFrame::SetSize(float width, float height)
{
    this->height = height;
    this->width = width;

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(scale_x, scale_y, 0));
    model = glm::translate(model, glm::vec3(-1.0f/scale_x + width + x, 1.0f/scale_y - y, 0));
    model = glm::scale(model, glm::vec3(width, height, 0));

    program.SetUniform(sp::kMatrix4fv, "model_matrix", glm::value_ptr(model));
}

void GUIFrame::SetPos(float x, float y)
{
    this->x = x;
    this->y = y;
    SetSize(width, height);
}

