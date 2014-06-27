#ifndef _SP_GEOMETRY_H_
#define _SP_GEOMETRY_H_

namespace sp {

struct BufferData {
    GLuint vao, vbo, ebo;

    ~BufferData() {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    }
};

void MakeCube(BufferData *buffer);
void MakePlane(BufferData *buffer);

} // namespace sp

#endif