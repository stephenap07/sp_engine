#include "console.h"


void Console::Init(float window_width, float window_height)
{
    frame.Init(0, 0, 2.0f/window_width, 2.0f/window_height, window_width, window_height / 2.0f);
    is_active = false;
    can_draw = false;
}

void Console::OpenFrame()
{
    is_active = true;
}

void Console::CloseFrame()
{
    is_active = false;
}

void Console::Update(float delta)
{
    if (is_active) {
        can_draw = true;
    } else {
        can_draw = false;
    }
}

void Console::Draw()
{
    if (can_draw) {
        frame.Draw();
    }
}
