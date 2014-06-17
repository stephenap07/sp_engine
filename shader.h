
namespace sp {
namespace shader {

struct Vertex {
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat tangent[4];
    GLfloat texcoord[2];
    GLubyte blendindex[4];
    GLubyte blendweight[4];
};

std::string ReadFileToString(const std::string &file_name);
GLuint CreateShader(GLenum eShaderType, const std::string &strShaderFile);
GLuint CreateProgram(const std::vector<GLuint> &kShaderList);

void SetVertAttribPointers();

} // namespace shader
} // namespace sp
