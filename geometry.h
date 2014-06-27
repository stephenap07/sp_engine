#ifndef _SP_GEOMETRY_H_
#define _SP_GEOMETRY_H_

namespace sp {

struct Renderable {
    GLuint vao, vbo, ebo;

    ~Renderable() {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    }
};

void MakeCube(Renderable *buffer);
void MakePlane(Renderable *buffer);

} // namespace sp

#endif
