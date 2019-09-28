#include "string_container.h"
#include <stdlib.h>

string_container *string_container_init(const char *src, size_t length)
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

string_container *string_container_copy(const string_container *str)
{
    if (str == NULL)
    {
        return NULL;
    }

    return string_container_init(str->src, str->length);
}

string_container *string_container_append(const string_container *str, const string_container *other_str)
{
    if (str == NULL && other_str == NULL)
    {
        return NULL;
    }
    else if (str == NULL && other_str)
    {
        return string_container_copy(other_str);
    }
    else if (str && other_str == NULL)
    {
        return string_container_copy(str);
    }

    size_t length = str->length + other_str->length;
    size_t buffer_size = length + 1;
    char *buffer = (char *)malloc(buffer_size);
    memset(buffer, '\0', buffer_size);

    memcpy(buffer, str->src, str->length);
    memcpy(&buffer[str->length], other_str->src, other_str->length);

    string_container *container = string_container_init(buffer, length);
    free(buffer);
    buffer = NULL;

    return container;
}