#ifndef STRING_ENCODER_H
#define STRING_ENCODER_H

#include <iconv.h>
#include "string_container.h"

class StringEncoder
{
public:
    StringEncoder(const char *from,const char *to);
    ~StringEncoder();

    string_container * convert(const char *inBuff,size_t inBuffLeft);
private:
    iconv_t handle;
};

#endif 
