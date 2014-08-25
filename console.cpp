#include "console.h"


namespace sp {

void Console::Init(float window_width, float window_height)
{
    width = window_width;
    height = window_height;

    frame.Init(0, 0, 2.0f/window_width, 2.0f/window_height, window_width, 0.0f);
    text_box.Init(0, 0, 2.0f/window_width, 2.0f/window_height, window_width, 15.0f);
    text_box.SetColor(glm::vec4(0.0, 0.0, 0.0, 0.8f));

    is_active = false;

    // Generating a whole new 3 texture atlases
    // TODO: Change this
    text_def.Init(window_width, window_height);
}

void Console::OpenFrame()
{
    is_active = true;
}

void Console::CloseFrame()
{
    is_active = false;
    frame.SetSize(width, 0.0f);
}

void Console::Update(float delta)
{
    if (!is_active) {
        return;
    }

    float frame_height = frame.GetHeight();
    float target_height = height / 2.0f;

    if (frame_height < target_height) {
        float incr = delta * target_height * 10.0f;
        frame.SetSize(width, frame_height + incr);
    } else if (frame_height > target_height) {
        frame.SetSize(width, target_height);
    }
    text_box.SetPos(0, frame_height - text_box.GetHeight());
}

void Console::Draw()
{
    if (is_active) {
        frame.Draw();
        text_box.Draw();
        text_def.DrawText(console_text, 0, frame.GetHeight() - 5.0f);
    }
}

void Console::SetText(const std::string &text)
{
    console_text = text;
}

const std::string Console::GetText() const
{
    return console_text;
}

} // namespace sp
