#ifndef _SP_SHADER_H_
#define _SP_SHADER_H_ 

#include <GL/glew.h>
#include <tuple>
#include <vector>
#include <tuple>
#include <string>

namespace sp {

enum GLUniformType{
    k1i,
    k4fv,
    k2fv,
    kMatrix4fv,
    kMatrix3x4fv,
};

struct Vertex {
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat tangent[4];
    GLfloat texcoord[2];
    GLubyte blendindex[4];
    GLubyte blendweight[4];
};

struct GLProgram {
    GLuint id;
};

namespace backend {
    void Bind(GLProgram);
    GLProgram CreateProgram(const std::vector<std::pair<const char*, GLenum>> &shader_pair);
    void SetUniform(GLProgram program, GLUniformType type, const char *name, GLvoid *data);
    void SetUniform(GLProgram program, GLUniformType type, const char *name, GLsizei count, GLvoid *data);
    void SetUniform(GLProgram program, GLUniformType type, const char *name, const GLint data);
    void FreeGLProgram(GLProgram program);
    void SetVertAttribPointers();
}

} // namespace sp

#endif
