#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <streambuf>

#include <GL/glew.h>

#include "shader.h"
#include "logger.h"

namespace sp {
namespace shader {

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

GLuint CreateShader(GLenum eShaderType, const std::string &shader_file_name)
{
    std::string contents = ReadFileToString(shader_file_name);

    GLuint shader = glCreateShader(eShaderType);
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
        switch(eShaderType)
        {
            case GL_VERTEX_SHADER: shader_type_cstr = "vertex"; break;
            case GL_GEOMETRY_SHADER: shader_type_cstr = "geometry"; break;
            case GL_FRAGMENT_SHADER: shader_type_cstr = "fragment"; break;
        }

        std::cerr << "Compile failure in "  << shader_type_cstr << " shader "
            << shader_file_name << ":\n" << str_info_log << std::endl;
        delete[] str_info_log;
    }

    return shader;
}

GLuint CreateProgram(const std::vector<GLuint> &kShaderList)
{
    GLuint program = glCreateProgram();

    for(size_t iLoop = 0; iLoop < kShaderList.size(); iLoop++)
        glAttachShader(program, kShaderList[iLoop]);

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }

    for(size_t iLoop = 0; iLoop < kShaderList.size(); iLoop++)
        glDetachShader(program, kShaderList[iLoop]);

    return program;
}

void SetVertAttribPointers()
{
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
    // Tangent
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
    // Texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(10 * sizeof(GLfloat)));
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

} // namespace shader
} // namespace sp
