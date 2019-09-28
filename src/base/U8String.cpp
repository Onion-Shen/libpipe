#include "U8String.h"

U8String::U8String()
{
    container = nullptr;
}

U8String::U8String(const char *src, size_t length)
{
    if (src == nullptr)
    {
        container = nullptr;
        return;
    }

    if (length == -1)
    {
        length = strlen(src);
    }

    container = string_container_init(src, length);
}

U8String::U8String(const U8String &str)
{
    container = string_container_copy(str.container);
}

U8String::~U8String()
{
    string_container_deinit(container);
}

size_t U8String::length()
{
    return container != nullptr ? container->length : 0;
}

size_t U8String::unicode_length()
{
    size_t length = 0;
    if (container == nullptr || container->length == 0)
    {
        return length;
    }

    size_t size = container->length;
    for (int i = 0; i < size; ++i)
    {
        char u8_ch = container->src[i];
        unsigned char ch = u8_ch;

        // 1byte 0xxxxxxx
        // 2byte 110xxxxx 10xxxxxx
        // 3byte 1110xxxx 10xxxxxx 10xxxxxx
        // 4byte 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        // 5byte 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        // 6byte 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        int count = -1;
        if ((ch & 0b01111111) == ch)
        {
            count = 1;
        }
        else if ((ch & 0b11011111) == ch)
        {
            count = 2;
        }
        else if ((ch & 0b11101111) == ch)
        {
            count = 3;
        }
        else if ((ch & 0b11110111) == ch)
        {
            count = 4;
        }
        else if ((ch & 0b11111011) == ch)
        {
            count = 5;
        }
        else if ((ch & 0b11111101) == ch)
        {
            count = 6;
        }

        if (count == -1)
        {
            //error ch
            continue;
        }

        length++;
        i += count - 1;
    }

    return length;
}

std::ostream &operator<<(std::ostream &stream, const U8String &str)
{
    if (str.container == nullptr || str.container->length == 0)
    {
        stream << "NULL";
    }
    else
    {
        stream << str.container->src;
    }
    return stream;
}

U8String operator+(const U8String &str, const U8String &otherStr)
{
    string_container *container = string_container_append(str.container, otherStr.container);
    if (!container)
    {
        return U8String();
    }

    auto newStr = U8String(container->src, container->length);
    string_container_deinit(container);

    return newStr;
}