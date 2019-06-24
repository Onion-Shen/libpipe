#include "string_container.h"
#include <stdlib.h>

string_container * string_container_init(const char *src, size_t length)
{
    string_container *container = NULL;
    if (src == NULL)
    {
        return container;
    }

    size_t str_size = length + 1;
    container = (string_container *)malloc(sizeof(string_container) + str_size);
    container->length = length;
    memset(container->src, '\0', str_size);
    memcpy(container->src, src, length);

    return container;
}

void string_container_deinit(string_container *str)
{
    if (str == NULL)
    {
        return;
    }

    free(str);
    str = NULL;
}