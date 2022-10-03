#include "sock.h"
#include <sys/socket.h>
#include <stdio.h>

bool send_all(int sockfd, const void *buf, size_t len, int flags)
{
    size_t pos = 0;
    char *buffer = buf;
    bool result = false;

    while (pos < len) {
        size_t remaining = len - pos;
        ssize_t n = send(sockfd, buffer + pos, remaining < 1024 ? remaining : 1024, flags);
        if (n == -1) {
            return result;
        }

        pos += n;
    }

    return true;
}
