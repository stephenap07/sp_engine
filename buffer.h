#ifndef _SP_BUFFER_H_
#define _SP_BUFFER_H_

namespace sp {

struct Point {
    GLfloat x, y, z, s, t;
};

class VertexBuffer
{
public:
    GLuint vao, vbo, ebo;

    VertexBuffer();
    ~VertexBuffer();

    void Bind();
    void Init();
    void DeleteBuffers();
};

void UnbindAllBuffers();
VertexBuffer MakeTexturedQuad(GLuint gl_hint=GL_TEXTURE_2D);
VertexBuffer MakeQuadWithTexcoord(float width, float height, float x, float y);

} // namespace sp

#endif
