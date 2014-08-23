#ifndef _SP_CONSOLE_H_
#define _SP_CONSOLE_H_

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 
#include "gui.h"

class Console {
public:
    Console() :width(0.0f), height(0.0f), is_active(false), can_draw(false)
    {}
    Console(float window_width, float window_height) :width(window_width),
                                                      height(window_height)
    {
        Init(width, height);
    }
    void Init(float window_width, float window_height);
    void OpenFrame();
    void CloseFrame();
    bool FrameIsOpen() const { return is_active; }
    void Update(float delta);
    void Draw();

private:
    GUIFrame frame;
    float width;
    float height;
    bool is_active;
    bool can_draw;
};

#endif
