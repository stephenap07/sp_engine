#ifndef _SP_CONSOLE_H_
#define _SP_CONSOLE_H_

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 
#include "gui.h"
#include "font.h"
#include "command.h"

namespace sp {

class Console {
public:
    Console() :
        width(0.0f),
        height(0.0f),
        is_active(false),
        draw_text_bar(false)
    {}

    Console(float window_width, float window_height) :
        width(window_width),
        height(window_height),
        is_active(false),
        draw_text_bar(false)
    {
        Init(width, height);
    }

    void Init(float window_width, float window_height);
    void Update(float delta);
    void Draw();

    void HandleEvent(const SDL_Event &sdl_event);
    void HandleCommand(const std::string &command_string);
    bool FrameIsOpen() const { return is_active; }

    void OpenFrame();
    void CloseFrame();

    void SetText(const std::string &text);
    const std::string GetText() const;

private:
    GUIFrame                 frame;
    GUIFrame                 text_box;
    TextDefinition           *text_def;

    std::string              console_text;
    std::vector<std::string> command_history;

    float                    width;
    float                    height;
    float                    text_bar_counter;
    bool                     is_active;
    bool                     draw_text_bar;
};

} // namespace sp

#endif
