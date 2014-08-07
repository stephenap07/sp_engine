#include <GL/glew.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

#include "util.h"
#include "buffer.h"
#include "geometry.h"

namespace sp {

struct GeomVertex {
    glm::vec3 vertex;
    glm::vec3 normal;
};

void MakeCube(VertexBuffer *buffer, bool has_normals)
{
    static GLuint vao;
    if (!vao) {
        glGenVertexArrays(1, &vao);
    }
    buffer->vao = vao;

    glGenBuffers(1, &buffer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
    glBindVertexArray(buffer->vao);
    glGenBuffers(1, &buffer->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ebo);

    const GLushort cube_indices[] =
    {
        0, 1, 2, 3, 6, 7, 4, 5,         // First strip
        0xFFFF,
        2, 6, 0, 4, 1, 5, 3, 7          // Second strip
    };

    /**************************************************
     (3) -------------_ (7)
        |`-           |`-
        |  `-         |  `-
        |    `-       |    `-
        |   (2)`------|------`- (6)
        |_______|_____|(5)     |
     (1)-       |      `-      |
         `-     |        `-    |
           `-   |          `-  |
             `- |            `-|
               `|______________|
            (0)               (4)
    **************************************************/

    if (!has_normals) {
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

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    } else {

        GeomVertex cube_vertices[] =
        {
            {.vertex = glm::vec3(-1.0f, -1.0f, -1.0f), .normal = glm::vec3(0.0f)},
            {.vertex = glm::vec3(-1.0f, -1.0f,  1.0f), .normal = glm::vec3(0.0f)},
            {.vertex = glm::vec3(-1.0f,  1.0f, -1.0f), .normal = glm::vec3(0.0f)},
            {.vertex = glm::vec3(-1.0f,  1.0f,  1.0f), .normal = glm::vec3(0.0f)},
            {.vertex = glm::vec3(1.0f, -1.0f, -1.0f),  .normal = glm::vec3(0.0f)},
            {.vertex = glm::vec3(1.0f, -1.0f,  1.0f),  .normal = glm::vec3(0.0f)},
            {.vertex = glm::vec3(1.0f,  1.0f, -1.0f),  .normal = glm::vec3(0.0f)},
            {.vertex = glm::vec3(1.0f,  1.0f,  1.0f),  .normal = glm::vec3(0.0f)}
        };

        for (size_t i = 0; i < 16; i++) {
            if (cube_indices[i] == 0xFFFF) {
                continue;
            } else if (i > 5) {
                continue;
            }

            glm::vec3 verts[3] = {
                cube_vertices[cube_indices[i]].vertex,
                cube_vertices[cube_indices[i+1]].vertex,
                cube_vertices[cube_indices[i+2]].vertex
            };

            glm::vec3 normal = glm::normalize(glm::cross(verts[2] - verts[0], verts[1] - verts[0]));

            for (size_t j = 0; j < 3; j++) {
                cube_vertices[cube_indices[i + j]].normal += normal;
            }
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GeomVertex), BUFFER_OFFSET(0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GeomVertex), BUFFER_OFFSET(sizeof(glm::vec3)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GeomVertex), cube_vertices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//=============================================================================

void MakeQuad(VertexBuffer *buffer)
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

void MakeTexturedQuad(VertexBuffer *buffer, GLuint gl_hint)
{
    struct Point {
        GLfloat x, y, z, s, t;
    };

    Point vert_quad[] = {
        // Position          // Texture Coordinates
        {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
        { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
        { 1.0f,  1.0f, 0.0f, 1.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f, 0.0f, 0.0f}
    };

    glGenVertexArrays(1, &buffer->vao);
    glBindVertexArray(buffer->vao);

    glGenBuffers(1, &buffer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Point), vert_quad, gl_hint);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (GLvoid*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


} // namespace sp
