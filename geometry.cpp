#include <GL/glew.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
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
    static GLuint vao_1 = 0;
    static GLuint vao_2 = 0;

    glGenBuffers(1, &buffer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);

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
        if (!vao_1) {
            glGenVertexArrays(1, &vao_1);
        }
        buffer->vao = vao_1;
        glBindVertexArray(buffer->vao);

        const GLfloat cube_vertices[] = {
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
        if (!vao_2) {
            glGenVertexArrays(1, &vao_2);
        }
        buffer->vao = vao_2;
        glBindVertexArray(buffer->vao);

        static const float kCubeVertexData[] = { 
            -1.0f,-1.0f,-1.0f,
            -1.0f,-1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f, 1.0f,-1.0f,
            1.0f,-1.0f, 1.0f,
            -1.0f,-1.0f,-1.0f,
            1.0f,-1.0f,-1.0f,
            1.0f, 1.0f,-1.0f,
            1.0f,-1.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f,-1.0f,
            1.0f,-1.0f, 1.0f,
            -1.0f,-1.0f, 1.0f,
            -1.0f,-1.0f,-1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f,-1.0f, 1.0f,
            1.0f,-1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f,-1.0f,-1.0f,
            1.0f, 1.0f,-1.0f,
            1.0f,-1.0f,-1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f,-1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f,-1.0f,
            -1.0f, 1.0f,-1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f,-1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f,-1.0f, 1.0f
        };
        
        glm::vec3 cube_vertices[sizeof(kCubeVertexData)/sizeof(float) / 3];
        for (size_t i=0; i < sizeof(kCubeVertexData)/sizeof(float); i+=3) {
            cube_vertices[i / 3] = glm::vec3(kCubeVertexData[i], kCubeVertexData[i + 1], kCubeVertexData[i + 2]);
        }

        std::vector<GeomVertex> new_vertices;

        for (size_t i = 0; i < 36; i++) {
            if (i % 3 == 2) {
                glm::vec3 verts[3] = {
                    cube_vertices[i-2],
                    cube_vertices[i-1],
                    cube_vertices[i]
                };
                
                //glm::vec3 f = verts[1]; 

                glm::vec3 normal = glm::normalize(glm::cross(verts[2] - verts[0], verts[1] - verts[0]));

                for (size_t j = 0; j < 3; j++) {
                    GeomVertex v = {.vertex = verts[j], .normal = normal};
                    new_vertices.push_back(v);
                }
            }
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GeomVertex), BUFFER_OFFSET(0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GeomVertex), BUFFER_OFFSET(sizeof(glm::vec3)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBufferData(GL_ARRAY_BUFFER, new_vertices.size() * sizeof(GeomVertex), &new_vertices[0], GL_STATIC_DRAW);
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
        1.0f, -1.0f, 1.0f,   // Bottom Right
        -1.0f, -1.0f, 1.0f,   // Bottom Left
        1.0f,  1.0f, -1.0f  // Top Right
    };

    GLushort vert_indices[] = {
        0, 2, 3, 1
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
