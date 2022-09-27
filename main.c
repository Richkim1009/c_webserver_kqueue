#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <sys/event.h>
#include <fcntl.h>
#include <unistd.h>

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

    EV_SET(change_list, server_sock, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kqfd, change_list, 1, NULL, 0, NULL);

    int new_events = 0;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;
    struct kevent curr_event;

    while (!server_stopped) {
        new_events = kevent(kqfd, NULL, 0, event_list, 8, 0);
        printf("%d\n", new_events);
        if (new_events == -1) {
            perror("kqueue()");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < new_events; ++i) {
            printf("New connection coming in...\n");
            curr_event = event_list[i];
            if (curr_event.ident == server_sock) {
                client_fd = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd == -1) {
                    perror("accept()");
                    exit(EXIT_FAILURE);
                }
                fcntl(client_fd, F_SETFL, O_NONBLOCK);
                EV_SET(change_list, client_fd, EVFILT_READ, EV_ADD, 0, 0, 0);
                kevent(kqfd, change_list, 1, NULL, 0, NULL);
            } else {
                if (curr_event.flags & EVFILT_READ) {
                    char *recv_buf = malloc(1024);
                    int bytes_read = recv(curr_event.ident, recv_buf, 1023, 0);
                    recv_buf[bytes_read] = '\0';
                    printf("%s\n", recv_buf);
                    if (strncmp(recv_buf, "GET", 3) == 0) {
                        const char * http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
                        send(curr_event.ident, http_response, strlen(http_response), 0);
                        curr_event.flags |= EV_EOF;
                        printf("%s\n", http_response);
                    }
                    free(recv_buf);
                }

                if (curr_event.flags & EV_EOF) {
                    EV_SET(change_list, curr_event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
//                    int err = kevent(kqfd, change_list, 1, NULL, 0, 0);

//                    if (err < 0)
//                    {
//                        printf("Error\n");
////                        continue;
//                    }

                    close(curr_event.ident);
                }
            }

//            char *str = malloc(1024);
//            ssize_t length = recv(client_fd, str, 1024, 0);
//            if (strncmp(str, "GET", 3) == 0) {
//
//                send(client_fd, http_response, strlen(http_response), 0);
//
//            }
        }
    }


    return EXIT_SUCCESS;
}
