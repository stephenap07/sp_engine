#include <iostream>
#include <cstring>

#include "Command.hpp"
#include "Logger.hpp"

namespace sp
{

namespace CommandManager
{
static std::unordered_map<std::string, tCommandFunction> commands;
}

void CommandManager::AddCommand(std::string name, tCommandFunction command)
{
    commands.insert(std::pair<std::string, tCommandFunction>(name, command));
}

bool CommandManager::FindAndExecute(std::string name, const CommandArg &args)
{
    auto cmd = commands.find(name);
    if (cmd != commands.end()) {
        cmd->second(args);
    } else {
        log::ErrorLog("command %s not found\n", name.c_str());
    }

    return true;
}

CommandArg::CommandArg(const char *text) { TokenizeString(text); }

void CommandArg::TokenizeString(const char *text)
{
    int command_length = strlen(text);
    int start_token = 0;
    char buffer[128];
    bool not_just_whitespace = false;

    for (int i = 0; i < command_length; i++) {
        if (text[i] == ' ' || i == (command_length - 1)) {
            if (not_just_whitespace) {
                int end_of_token = i - start_token + 1;
                strncpy(buffer, text + start_token, end_of_token);
                if (buffer[end_of_token - 1] == ' ') {
                    buffer[end_of_token - 1] = '\0';
                } else {
                    buffer[end_of_token] = '\0';
                }
                argv.push_back(std::string(buffer));

                memset(buffer, 0, sizeof(buffer));
            }
            start_token = i + 1;
        } else {
            not_just_whitespace = true;
        }
    }
}

static const std::string kEmptyString = "";

const std::string &CommandArg::GetArg(unsigned int place) const
{
    if (place < argv.size()) {
        return argv[place];
    } else {
        return kEmptyString;
    }
}

const int CommandArg::Argc() const { return argv.size(); }

} // namespace sp
