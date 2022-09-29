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
#include <stdarg.h>
#include "str.h"
#include "sock.h"
#include "http.h"

static const int back_log = 32;
static const bool server_stopped = false;
static const int port = 8080;
static const int max_changes = 8;
static const int recv_buf_capacity = 2048;

void log_debug(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

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
                    char *recv_buf = malloc(recv_buf_capacity);
                    int bytes_read = recv(curr_event.ident, recv_buf, recv_buf_capacity - 1, 0);
                    recv_buf[bytes_read] = '\0';
                    struct HttpRequest client;
                    char **token = parse_http_header(recv_buf, "\n", " ");
                    struct MethodTableEntry method_table[] = {
                            { "GET", HTTP_METHOD_GET },
                            { "HEAD", HTTP_METHOD_HEAD },
                            { "POST", HTTP_METHOD_POST },
                            { "PUT", HTTP_METHOD_PUT },
                            { "DELETE", HTTP_METHOD_DELETE },
                            { "TRACE", HTTP_METHOD_TRACE },
                            { "OPTIONS", HTTP_METHOD_OPTIONS },
                            { "CONNECT", HTTP_METHOD_CONNECT },
                            { "PATCH", HTTP_METHOD_PATCH }
                    };

                    int found_http_method = -1;

                    for (int i = 0; i < 9; ++i) {
                        if (strncmp(token[0], method_table[i].request_line_prefix, strlen(token[0])) == 0) {
                            found_http_method = i;
                            break;
                        }
                    }

                    if (found_http_method == 0) {
                        const char *file_path = server_file_path(token[1]);
                        FILE *fp = fopen(file_path, "r");
                        if (fp == NULL) {
                            perror("fopen()");
                            exit(EXIT_FAILURE);
                        }
                        if (fseek(fp, 0, SEEK_END) == -1) {
                            perror("fseek()");
                            exit(EXIT_FAILURE);
                        }
                        long file_size = ftell(fp);

                        if (file_size == -1) {
                            perror("ftell()");
                            fclose(fp);
                            exit(EXIT_FAILURE);
                        }

                        if (fseek(fp, 0, SEEK_SET)) {
                            perror("fseek()");
                            fclose(fp);
                            exit(EXIT_FAILURE);
                        }

                        char *file_content = malloc(file_size);

                        size_t n = fread(file_content, 1, file_size, fp);

                        if (n != file_size) {
                            log_debug("Error while reading the file: n != file_size\n");
                            fclose(fp);
                            exit(EXIT_FAILURE);
                        }

                        fclose(fp);

                        char *http_response_first = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

                        send(curr_event.ident, http_response_first, strlen(http_response_first), 0);
                        bool result = send_all(curr_event.ident, file_content, n, 0);
                        if (!result) {
                            perror("send()");
                            exit(EXIT_FAILURE);
                        }
                        curr_event.flags |= EV_EOF;
                    }
                    free(recv_buf);
                }

                if (curr_event.flags & EV_EOF) {
                    EV_SET(change_list, curr_event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    close(curr_event.ident);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
