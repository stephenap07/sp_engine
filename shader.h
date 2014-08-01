#ifndef _SP_SHADER_H_
#define _SP_SHADER_H_ 
#include <GL/glew.h>
#include <tuple>
#include <vector>
#include <tuple>
#include <string>

namespace sp {

enum GLUniformType{
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

class Shader
{
public:
    Shader(): id(0)
    {}

    Shader(const std::vector<std::pair<const std::string &, GLenum>> &shader_pair);
    void CreateProgram(const std::vector<std::pair<const std::string &, GLenum>> &shader_pair);
    ~Shader();

    void Bind();
    void SetUniform(GLUniformType type, const char *name, GLvoid *data);
    void SetUniform(GLUniformType type, const char *name, GLsizei count, GLvoid *data);
    GLuint GetID() const { return id; }

private:
    GLuint id;
};

std::string ReadFileToString(const std::string &file_name);
GLuint CreateProgram(const std::vector<GLuint> &kShaderList);
void SetVertAttribPointers();

} // namespace sp

#endif
