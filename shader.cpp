#include <GL/glew.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <streambuf>
#include <algorithm>

#include "shader.h"
#include "logger.h"
#include "error.h"

namespace sp {

//------------------------------------------------------------------------------

std::string ReadFileToString(const std::string &file_name)
{
    std::ifstream file(file_name);

    if (!file.good()) {
        log::ErrorLog("Error opening file %s\n", file_name.c_str());
    }

    std::string contents;
    file.seekg(0, std::ios::end);
    contents.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    // Extra parenthesis necessary for vexing parse
    contents.assign((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

    return contents;
}

//------------------------------------------------------------------------------

GLuint CreateShader(const std::string &shader_file_name, GLenum target)
{
    std::string contents = ReadFileToString(shader_file_name);

    GLuint shader = glCreateShader(target);
    const char *contents_cstr = contents.c_str();
    glShaderSource(shader, 1, &contents_cstr, NULL);

    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint info_log_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);

        GLchar *str_info_log = new GLchar[info_log_len + 1];
        glGetShaderInfoLog(shader, info_log_len, NULL, str_info_log);

        const char *shader_type_cstr = NULL;
        switch(target)
        {
            case GL_VERTEX_SHADER: shader_type_cstr = "vertex"; break;
            case GL_GEOMETRY_SHADER: shader_type_cstr = "geometry"; break;
            case GL_FRAGMENT_SHADER: shader_type_cstr = "fragment"; break;
        }

        fprintf(stderr, "Compile failure in %s shader %s:\n %s\n",
                      shader_type_cstr, shader_file_name.c_str(), str_info_log);
        delete[] str_info_log;
    }

    return shader;
}

//------------------------------------------------------------------------------

static GLuint LocalCreateProgram(const std::vector<GLuint> &kShaderList)
{
    GLuint program = glCreateProgram();
    for (auto s : kShaderList) {
        glAttachShader(program, s);
    }
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint info_log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

        GLchar *str_info_log = new GLchar[info_log_length + 1];
        glGetProgramInfoLog(program, info_log_length, NULL, str_info_log);
        fprintf(stderr, "Linker failure: %s\n", str_info_log);
        delete[] str_info_log;
    }

    return program;
}

//------------------------------------------------------------------------------

void SetVertAttribPointers()
{
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
    // Texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(10 * sizeof(GLfloat)));
    // Tangent
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
    // Blend Index
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (GLvoid*)(12 * sizeof(GLfloat)));
    // Blend Weight
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (GLvoid*)(12 * sizeof(GLfloat) + 4 * sizeof(GLubyte)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
}

Shader::Shader(const std::vector<std::pair<const std::string &, GLenum>> &shader_pair)
{
    CreateProgram(shader_pair);
}

void Shader::CreateProgram(const std::vector<std::pair<const std::string &, GLenum>> &shader_pair)
{
    std::vector<GLuint> shaders;
    for (auto p : shader_pair) {
        shaders.push_back(CreateShader(p.first, p.second));
    }

    id = LocalCreateProgram(shaders);
    
    if (id == 0) {
        std::cerr << "Invalid program\n";
    }

    /*
    std::for_each(shaders.begin(), shaders.end(),
            [id](GLuint shader_id) { glDetachShader(id, shader_id); });
            */
}

void Shader::Bind()
{
    glUseProgram(id);
}

void Shader::SetUniform(GLUniformType type, const char *name, GLvoid *data)
{
    SetUniform(type, name, 1, data);
}

void Shader::SetUniform(GLUniformType type, const char *name, const std::vector<GLint> &data)
{
    Bind();

    GLint uniform = glGetUniformLocation(id, name);
    if (uniform == -1) {
        std::cerr << "Invalid uniform for " << name << std::endl;
    } else {
        switch (type) {
            case k1i:
                glUniform1i(uniform, data[0]);
            default:
                std::cerr << "Invalid uniform type\n";
                break;
        }
    }
}

void Shader::SetUniform(GLUniformType type, const char *name, GLsizei count, GLvoid *data)
{
    Bind();

    GLint uniform = glGetUniformLocation(id, name);
    if (uniform == -1) {
        std::cerr << "Invalid uniform for " << name << std::endl;
    } else {
        switch (type) {
            case k4fv:
                glUniform4fv(uniform, count, (GLfloat*)data); break; case k2fv:
                glUniform2fv(uniform, count, (GLfloat*)data);
                break;
            case kMatrix4fv:
                glUniformMatrix4fv(uniform, count, GL_FALSE, (GLfloat*)data);
                break;
            case kMatrix3x4fv:
                glUniformMatrix3x4fv(uniform, count, GL_FALSE, (GLfloat*)data);
            default:
                std::cerr << "Invalid uniform type\n";
                break;
        }
    }
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

} // namespace sp
