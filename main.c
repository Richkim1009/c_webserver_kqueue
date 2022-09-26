#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/event.h>
#include <pthread.h>
#include <sys/time.h>

static const int back_log = 32;
static const bool server_stopped = false;
static const int port = 8080;
static const int max_changes = 8;

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sock == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    int so_reuseaddr_enable = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr_enable, sizeof(int)) == -1) {
        perror("setsocketopt()");
        exit(1);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof server_addr) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, back_log) == -1) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    int kqfd = kqueue();

    if (kqfd == -1) {
        perror("kqueue()");
        exit(EXIT_FAILURE);
    }

    struct kevent change_list[max_changes];
    struct kevent event_list[max_changes];

    EV_SET(change_list, server_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    if (kevent(kqfd, change_list, 1, NULL, 0, NULL) == -1)
    {
        perror("kevent()");
        exit(EXIT_FAILURE);
    }

    int new_events = 0;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (!server_stopped) {
        new_events = kevent(kqfd, NULL, 0, event_list, 1, NULL);
        if (new_events == -1) {
            perror("kqueue()");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < new_events; ++i) {
            printf("New connection coming in...\n");

            int client_fd = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_fd == -1) {
                perror("accept()");
                exit(EXIT_FAILURE);
            }

            printf("client_fd: %d\n", client_fd);
            char *str = malloc(1024);
            char *tmp;
            strncpy(tmp, str, 3);
            ssize_t length = recv(client_fd, str, 1024, 0);
            printf("%s\n", tmp);
            if (strncmp(tmp, "GET", 3)) {
                const char * http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
                send(client_fd, http_response, strlen(http_response), 0);

            }
        }
    }


    return EXIT_SUCCESS;
}
