
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

typedef std::pair<GLenum, std::string> ShaderPair;

std::string ReadFileToString(const std::string &file_name);
GLuint CreateShader(ShaderPair shader_pair);
GLuint CreateProgram(const std::vector<GLuint> &kShaderList);

void SetVertAttribPointers();

} // namespace shader
} // namespace sp
