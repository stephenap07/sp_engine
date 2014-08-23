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
    void Draw();

private:
    sp::VertexBuffer buffer;
    sp::Shader program;
    float width, height;
    float x, y;
    float scale_x, scale_y;
};

#endif
