#ifndef STRING_CONTAINER_H
#define STRING_CONTAINER_H

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    size_t length;
    char src[];
} string_container;

extern string_container * string_container_init(const char *src,size_t length);

extern void string_container_deinit(string_container *str);

extern string_container * string_container_copy(const string_container *str);

extern string_container * string_container_append(const string_container *str,const string_container *other_str);

#ifdef __cplusplus
}
#endif

#endif
