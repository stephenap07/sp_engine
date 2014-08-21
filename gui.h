#ifndef _SP_GUI_H_
#define _SP_GUI_H_

#include "shader.h"
#include "buffer.h"

class GUIFrame {
public:
    GUIFrame(float width, float height);
    ~GUIFrame();
    void Init();
    void Draw();

private:
    sp::VertexBuffer buffer;
    sp::Shader program;
    float width, height;
};

#endif
