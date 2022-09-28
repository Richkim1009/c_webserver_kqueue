#include "str.h"
#include <string.h>


bool str_starts_with(char *str, char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool str_ends_with(char *str, char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    return str_len < suffix_len ? false : strncmp(str + str_len - suffix_len + 1, suffix + 1, suffix_len - 1) == 0;
}

void remove_crlf(char *str)
{
    int len = strlen(str);
    if (str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
    if (str[len - 2] == '\r') {
        str[len - 2] = '\0';
    }
}
