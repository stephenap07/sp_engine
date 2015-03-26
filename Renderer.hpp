#ifndef _SP_RENDERER_H_
#define _SP_RENDERER_H_

#include <GL/glew.h>
#include <vector>

#include "GLProgram.hpp"

namespace sp {

class Renderer {
public:
    ~Renderer();

    void Init();
    void BeginFrame();
    void EndFrame();
    void FreeResources();
    void SetView(const glm::mat4 &view);
    void SetAngleOfView(const float angle);
    void LoadGlobalUniforms(GLuint shader_index);

    int GetWidth() const { return screen_width; }
    int GetHeight() const { return screen_height; }

    float GetScaleWidth(float width) const
    {
        return width * 2.0f / screen_width;
    }
    float GetScaleHeight(float height) const
    {
        return height * 2.0f / screen_height;
    }

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
};

class ViewEntity {
private:
    glm::mat4 mat4_model;
    glm::mat4 mat4_model_view;
};

class ViewDefinition {
public:

private:
    glm::mat4 mat4_view;
    glm::mat4 mat4_projection;
    std::vector<ViewEntity> view_entities;
};

} // namespace sp

#endif
