
namespace sp {

std::string ReadFileToString(const std::string &file_name);
GLuint CreateShader(GLenum eShaderType, const std::string &strShaderFile);
GLuint CreateProgram(const std::vector<GLuint> &kShaderList);

}
