#include "str.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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

char **parse_http_header(char *str, const char *first, const char *second)
{
    char *header = strtok(str, first);
    unsigned int count = 0;
    char *tmp = strtok(header, second);
    char **token = (char**)malloc(sizeof(char*) * 3);
    while (tmp != NULL) {
        token[count++] = tmp;
        tmp = strtok(NULL, second);
    }
    return token;
}

char *server_file_path(char *path)
{
    const char *basic_path = "../contents";
    char *file_path;
    char *index = strlen(path) == 1 ? "/index.html" : path;
    file_path = malloc(strlen(basic_path) + strlen(index) + 1);
    strcpy(file_path, basic_path);
    strcat(file_path, index);
    file_path[strlen(file_path)] = '\0';

    return file_path;
}
