#include <cstdarg>
#include <cstdio>
#include "Logger.hpp"

namespace sp {
namespace log {

void ErrorLog(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

void InfoLog(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
}

} // namespace log
} // namespace sp
