#include <GL/glew.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <memory>
#include <cassert>

#include "Shader.hpp"
#include "Logger.hpp"
#include "Error.hpp"

namespace sp {

static std::string ReadFileToString(const char* file_name);
static GLuint CreateShader(const char *shader_file_name, GLenum target);

//------------------------------------------------------------------------------

std::string ReadFileToString(const char *file_name)
{
    std::ifstream file(file_name);

    if (!file.good()) {
        log::ErrorLog("Error opening file %s\n", file_name);
		assert(false);
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

GLuint CreateShader(const char *shader_file_name, GLenum target)
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
        shader_type_cstr, shader_file_name, str_info_log);
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

namespace backend {
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

    GLProgram CreateProgram(const std::vector<std::pair<const char*, GLenum> > &shader_pair)
    {
        GLProgram program;
        std::vector<GLuint> shaders;
        for (auto p : shader_pair) {
            shaders.push_back(CreateShader(p.first, p.second));
        }

        program.id = LocalCreateProgram(shaders);

        if (program.id == 0) {
            std::cerr << "Invalid program\n";
        }

        return program;
    }

    void Bind(GLProgram program)
    {
        glUseProgram(program.id);
    }

    void SetUniform(GLProgram program, GLUniformType type, const char *name, GLvoid *data)
    {
        SetUniform(program, type, name, 1, data);
    }

    inline GLint GetGLUniformLocation(GLProgram program, const char *name)
    {
        glGetError();
        GLint uniform = glGetUniformLocation(program.id, name);
        GLenum error = glGetError();
        if (error) {
            log::ErrorLog("Error getting uniform %s from program %d\n", name, program.id);
            HandleGLError(error);
        }

        return uniform;
    }

    void SetUniform(GLProgram program, GLUniformType type, const char *name, const GLint data)
    {
        Bind(program);
        GLint uniform = GetGLUniformLocation(program, name);

        if (uniform == -1) {
            std::cerr << "Invalid uniform type (" << name << ") for program id " << program.id << std::endl;
        } else {
            switch (type) {
                case k1i:
                    glUniform1i(uniform, data);
                    break;
                default:
                    std::cerr << "Invalid uniform type (" << name << ") for program id " << program.id << std::endl;
                    break;
            }
        }
    }

    void SetUniform(GLProgram program, GLUniformType type, const char *name, GLsizei count, GLvoid *data)
    {
        Bind(program);
        GLint uniform = GetGLUniformLocation(program, name);

        if (uniform == -1) {
            std::cerr << "Invalid uniform type (" << name << ") for shader id " << program.id << std::endl;
        } else {
            switch (type) {
                case k4fv:
                    glUniform4fv(uniform, count, (GLfloat*)data);
                    break;
                case k2fv:
                    glUniform2fv(uniform, count, (GLfloat*)data);
                    break;
                case kMatrix4fv:
                    glUniformMatrix4fv(uniform, count, GL_FALSE, (GLfloat*)data);
                    break;
                case kMatrix3x4fv:
                    glUniformMatrix3x4fv(uniform, count, GL_FALSE, (GLfloat*)data);
                    break;
                default:
                    std::cerr << "Invalid uniform type\n";
                    break;
            }
        }
    }

    void FreeGLProgram(GLProgram program) {
        glDeleteProgram(program.id);
    }

} // namespace backend
} // namespace sp
