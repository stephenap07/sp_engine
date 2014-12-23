#include <GL/glew.h>
#include "logger.h"
#include "error.h"

namespace sp {

void HandleGLError(GLenum error)
{
    switch (error) {
        case GL_NO_ERROR:
            break;
        case GL_INVALID_ENUM:
            log::ErrorLog("GL_ERROR: Invalid enum\n");
            break;
        case GL_INVALID_VALUE:
            log::ErrorLog("GL_ERROR: Invalid value\n");
            break;
        case GL_INVALID_OPERATION:
            log::ErrorLog("GL_ERROR: Invalid operation\n");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            log::ErrorLog("GL_ERROR: Invalid framebuffer operation\n");
            break;
        case GL_OUT_OF_MEMORY:
            log::ErrorLog("GL_ERROR: Out of memory\n");
            break;
        case GL_STACK_UNDERFLOW:
            log::ErrorLog("GL_ERROR: Stack underflow\n");
            break;
        case GL_STACK_OVERFLOW:
            log::ErrorLog("GL_ERROR: Stack overflow\n");
            break;
        default:
            break;
    };
}

} // namespace sp
