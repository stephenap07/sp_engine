#include <SDL2/SDL.h>
#include "console.h"
#include "logger.h"


namespace sp {

void Console::Init(float window_width, float window_height)
{
    width = window_width;
    height = window_height;

    frame.Init(0, 0, 2.0f/window_width, 2.0f/window_height, window_width, 0.0f);
    text_box.Init(0, 0, 2.0f/window_width, 2.0f/window_height, window_width, 15.0f);
    text_box.SetColor(glm::vec4(0.0, 0.0, 0.0, 0.4f));

    is_active = false;

    // Generating a whole new 3 texture atlases
    // TODO: Change this
    // text_def.Init(window_width, window_height);
    text_def = font::GetTextDef("SPFont.ttf");
}

//=============================================================================

void Console::OpenFrame()
{
    is_active = true;
}

//=============================================================================

void Console::CloseFrame()
{
    is_active = false;
    frame.SetSize(width, 0.0f);
}

//=============================================================================

void Console::Update(float delta)
{
    if (!is_active) {
        return;
    }

    text_bar_counter += delta;
    if (text_bar_counter >= 0.3f) {
        text_bar_counter = 0.0f;
        draw_text_bar = !draw_text_bar;
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

//=============================================================================

void Console::Draw()
{
    if (is_active) {
        frame.Draw();
        text_box.Draw();
		std::string label = console_text;
        if (draw_text_bar) {
            label += "|";
        }
        text_def->DrawText(label, 0, frame.GetHeight() - 5.0f);
        int cmd_count = command_history.size();
        for (int i = 0; i < cmd_count; i++) {
            text_def->DrawText(command_history[i], 0, frame.GetHeight() - 35.0f * (cmd_count - i));
        }
    }
}

//=============================================================================

void Console::SetText(const std::string &text)
{
    console_text = text;
}

//=============================================================================

const std::string Console::GetText() const
{
    return console_text;
}

//=============================================================================

void Console::HandleEvent(const SDL_Event &sdl_event)
{
    SDL_Keymod key_mod = SDL_GetModState();

    switch(sdl_event.type) {
        case SDL_KEYDOWN:
            if (sdl_event.key.keysym.sym == SDLK_BACKSPACE) {
                std::string console_text = GetText();
                SetText(console_text.substr(0, console_text.length() - 1));
            }
            if (sdl_event.key.keysym.sym == SDLK_RETURN) {
                std::string command_string = GetText();
                command_history.push_back(command_string);
                HandleCommand(command_string);
                SetText("");
            }
            if (key_mod & KMOD_CTRL) {
                if (sdl_event.key.keysym.sym == SDLK_a) {
                }
            }
            break;
        case SDL_TEXTINPUT:
            if (strcmp(sdl_event.text.text, "`") != 0) {
                SetText(GetText() + sdl_event.text.text);
            }
            break;
        case SDL_TEXTEDITING:
            log::InfoLog("SDL Text editing event\n");
            break;
    }
}

//=============================================================================

void Console::HandleCommand(const std::string &command_string)
{
    CommandArg args(command_string.c_str());
    bool success = CommandManager::FindAndExecute(args.GetArg(0), args);
    if (!success) {
        log::ErrorLog("Invalid command\n");
    }
}

} // namespace sp
