#ifndef _SP_GUI_H_
#define _SP_GUI_H_

#include "shader.h"
#include "buffer.h"

class GUIDef {
    glm::vec2 position;
    glm::vec2 scale;
};

class GUIFrame {
public:
    GUIFrame() {}
    GUIFrame(float x, float y, float sx, float sy, float width, float height);
    ~GUIFrame();
    void Init(float x, float y, float sx, float sy, float width, float height);
    void SetPos(float x, float y);
    void SetSize(float width, float height);
    void SetColor(const glm::vec4 &color);
    void Draw();

    float GetHeight() const { return height; };

private:

    sp::VertexBuffer buffer;
    sp::GLProgram program;
    glm::vec4 color;
    float width, height;
    float x, y;
    float scale_x, scale_y;
};

#endif
