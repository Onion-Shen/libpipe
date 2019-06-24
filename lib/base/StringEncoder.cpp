#include "StringEncoder.h"
#include <cstring>
#include <errno.h>
#include <iostream>

StringEncoder::StringEncoder(const char *from, const char *to)
{
    handle = iconv_open(to, from);
    if (handle == iconv_t(-1))
    {
        const char *error = strerror(errno);
        std::cerr << error << std::endl;
    }
}

StringEncoder::~StringEncoder()
{
    if (handle)
    {
        iconv_close(handle);
    }
}

string_container *StringEncoder::convert(const char *inBuff, size_t inBuffLeft)
{
    string_container *out = nullptr;
    if (handle == nullptr || inBuff == nullptr || inBuffLeft == 0)
    {
        return out;
    }

    char *inBuf = const_cast<char *>(inBuff);
    size_t inBufLeft = inBuffLeft;

    size_t outBufLeft = inBufLeft * 10;
    size_t outBufSize = outBufLeft;
    char *outBuf = new char[outBufLeft]();
    char *outBufAddr = outBuf;

    size_t result = iconv(handle, &inBuf, &inBufLeft, &outBuf, &outBufLeft);
    if (result == -1)
    {
        const char *error = strerror(errno);
        std::cerr << error << std::endl;
        return out;
    }

    out = string_container_init(outBufAddr, outBufSize - outBufLeft);
    delete[] outBufAddr;

    return out;
}