#include <GL/glew.h>

namespace sp {

inline void HandleGLError(GLenum error)
{
    switch (error) {
        case GL_NO_ERROR:
            break;
        case GL_INVALID_ENUM:
            std::cerr << "GL_ERROR: Invalid enum\n";
            break;
        case GL_INVALID_VALUE:
            std::cerr << "GL_ERROR: Invalid value\n";
            break;
        case GL_INVALID_OPERATION:
            std::cerr << "GL_ERROR: Invalid operation\n";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            std::cerr << "GL_ERROR: Invalid framebuffer operation\n";
            break;
        case GL_OUT_OF_MEMORY:
            std::cerr << "GL_ERROR: Out of memory\n";
            break;
        case GL_STACK_UNDERFLOW:
            std::cerr << "GL_ERROR: Stack underflow\n";
            break;
        case GL_STACK_OVERFLOW:
            std::cerr << "GL_ERROR: Stack overflow\n";
            break;
        default:
            break;
    };
}

} // namespace sp
