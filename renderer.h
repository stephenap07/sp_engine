#ifndef _SP_RENDERER_H_
#define _SP_RENDERER_H_

#include <GL/glew.h>
#include <vector>

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
    void LoadGlobalUniforms(GLuint shader_index);

    int GetWidth() const { return screen_width; }
    int GetHeight() const { return screen_height; }
    glm::mat4 GetView() const { return view; }

private:
    SDL_Window *window;
    SDL_GLContext context;

    glm::mat4 view;
    glm::mat4 projection;

    GLuint global_ubo;
    GLuint global_uniform_binding;

    int screen_width;
    int screen_height;

    std::vector<ProgramData> program_store;
};

} // namespace sp

#endif
