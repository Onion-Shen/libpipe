#ifndef UTIL_DEF_H
#define UTIL_DEF_H

// #define Util_Debug

#include <exception>

#define throwError(msg) throw std::logic_error(msg)

enum class HTTPMessageParseState : int
{
    Line = 0,
    Header,
    Body
};
namespace Util
{
    using byte = unsigned char;
};

#endif 
