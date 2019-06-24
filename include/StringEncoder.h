#ifndef STRING_ENCODER
#define STRING_ENCODER

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
