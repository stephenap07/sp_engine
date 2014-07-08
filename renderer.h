#ifndef _SP_RENDERER_H_
#define _SP_RENDERER_H_

#include "gl_program.h"

namespace sp {

class Renderer {
public:
    ~Renderer();

    void Init();
    void BeginFrame();
    void EndFrame();
    void FreeResources();
    void SetView(const glm::mat4 &view);
    ProgramData LoadProgram(const std::vector<GLuint> &kShaderList);

private:
    SDL_Window *window;
    SDL_GLContext context;

    glm::mat4 view;
    glm::mat4 projection;

    GLuint global_ubo;
    GLuint global_uniform_binding;

    int screen_width;
    int screen_height;
};

} // namespace sp

#endif
