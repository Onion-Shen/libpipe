#ifndef U8_STRING_H
#define U8_STRING_H

#include <ostream>
#include "string_container.h"

class U8String 
{
private:
    string_container *container;
public:
    U8String();
    U8String(const char *src,size_t length = -1);
    U8String(const U8String &str);

    ~U8String();
public:
    size_t length();
    size_t unicode_length();

    friend std::ostream & operator << (std::ostream &stream,const U8String &str);
    friend U8String operator + (const U8String &str,const U8String &otherStr);
};

#endif 
