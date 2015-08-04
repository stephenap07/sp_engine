#include "VertexBuffer.hpp"

namespace sp {

void UnbindAllBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//------------------------------------------------------------------------------

VertexBuffer::VertexBuffer() :vao(0), vbo(0), ebo(0), num_triangles(0)
{}

//------------------------------------------------------------------------------

VertexBuffer::~VertexBuffer()
{
    //glDeleteBuffers(1, &vbo);
}

//------------------------------------------------------------------------------

void VertexBuffer::Bind()
{
    glBindVertexArray(vao);
    if (vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }
    if (ebo) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }
}

//------------------------------------------------------------------------------

void VertexBuffer::Init()
{
    //glDeleteBuffers(1, &vbo);
}

void VertexBuffer::DeleteBuffers()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

VertexBuffer MakeTexturedQuad(GLuint gl_hint)
{
    Point vert_quad[] = {
        // Position          // Texture Coordinates
        {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
        { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
        { 1.0f,  1.0f, 0.0f, 1.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f, 0.0f, 0.0f}
    };

    VertexBuffer buffer;

    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Point), vert_quad, gl_hint);

    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (GLvoid*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return buffer;
}


VertexBuffer MakeQuadWithTexcoord(float width, float height, float x, float y)
{
    VertexBuffer buffer;
    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);
    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    Point vert_quad[] = {
        // Position          // Texture Coordinates
        {-1.0f,  1.0f, 0.0f, x + 0.0f, y + 0.0f},
        {-1.0f, -1.0f, 0.0f, x + 0.0f, y + height},
        { 1.0f, -1.0f, 0.0f, x + width, y + height},
        { 1.0f,  1.0f, 0.0f, x + width, y + 0.0f}
    };

    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Point), vert_quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (GLvoid*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return buffer;
}

} // namespace sp
