#include <GL/glew.h>

#include "util.h"
#include "geometry.h"

namespace sp {

void MakeCube(Renderable *buffer)
{
    static GLuint vao;
    if (!vao) {
        glGenVertexArrays(1, &vao);
    }
    buffer->vao = vao;

    glGenBuffers(1, &buffer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);

    const GLfloat cube_vertices[] =
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

    const GLushort cube_indices[] =
    {
        0, 1, 2, 3, 6, 7, 4, 5,         // First strip
        2, 6, 0, 4, 1, 5, 3, 7          // Second strip
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindVertexArray(buffer->vao);
    glBindVertexArray(buffer->vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &buffer->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//=============================================================================

void MakeQuad(Renderable *buffer)
{
    GLfloat vert_quad[] = {
        -1.0f,  1.0f, -1.0f, // Top Left
        1.0f,  1.0f, -1.0f,  // Top Right
        1.0f, -1.0f, 1.0f,   // Bottom Right
        -1.0f, -1.0f, 1.0f   // Bottom Left
    };

    GLuint vert_indices[] = {
        3, 2, 1,
        1, 0, 3
    };

    glGenBuffers(1, &buffer->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vert_indices), vert_indices,
            GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &buffer->vao);
    glBindVertexArray(buffer->vao);
    glGenBuffers(1, &buffer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vert_quad), vert_quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//=============================================================================

void MakeTexturedQuad(Renderable *buffer)
{
    GLfloat vert_quad[] = {
        // Position
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,

        // Texture Coordinates
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    glGenVertexArrays(1, &buffer->vao);
    glBindVertexArray(buffer->vao);

    glGenBuffers(1, &buffer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert_quad), vert_quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(12 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


} // namespace sp
