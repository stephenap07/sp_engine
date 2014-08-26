#ifndef _SP_CONSOLE_H_
#define _SP_CONSOLE_H_

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 
#include "gui.h"
#include "font.h"

namespace sp {

class Console {
public:
    Console() :width(0.0f), height(0.0f), is_active(false), draw_text_bar(false)
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
    void SetText(const std::string &text);
    const std::string GetText() const;

private:
    GUIFrame frame;
    GUIFrame text_box;
    TextDefinition text_def;

    std::string console_text;

    float width;
    float height;
    float text_bar_counter;
    bool is_active;
    bool draw_text_bar;
};

} // namespace sp
#endif
