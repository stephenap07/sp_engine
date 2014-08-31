#ifndef _SP_COMMAND_H_
#define _SP_COMMAND_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sp {

class CommandArg {
public:
    CommandArg() {}
    CommandArg(const char *text);
    const std::string &GetArg(unsigned int place) const;
    const int Argc() const;
private:
    void TokenizeString(const char *text);

    std::vector<std::string> argv;
};


typedef std::function<void(const CommandArg &)> tCommandFunction;

namespace CommandManager {
    void AddCommand(std::string name, tCommandFunction command);
    bool FindAndExecute(std::string name, const CommandArg &args);
}

} // namespace sp

#endif
