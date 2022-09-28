#ifndef C_WEBSERVER_KQUEUE_STR_H
#define C_WEBSERVER_KQUEUE_STR_H
#include <stdbool.h>
extern bool str_starts_with(char *str, char *prefix);
extern bool str_ends_with(char *str, char *suffix);
extern void remove_crlf(char *str);
extern char **parse_http_header(char *str, const char *first, const char *second);
#endif //C_WEBSERVER_KQUEUE_STR_H
