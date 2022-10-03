#ifndef C_WEBSERVER_KQUEUE_SOCK_H
#define C_WEBSERVER_KQUEUE_SOCK_H
#include <string.h>
#include <stdbool.h>

bool send_all(int sockfd, const void *buf, size_t len, int flags);
#endif //C_WEBSERVER_KQUEUE_SOCK_H
