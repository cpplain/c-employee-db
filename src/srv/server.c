#include "server.h"
#include "file.h"

#include "../common.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 256
#define BACKLOG 16

typedef char buffer[BUF_SIZE];
typedef char buffers[MAX_CLIENTS + 1][sizeof(buffer)];

void reset_state(struct pollfd *fd, buffer *buf) {
    fd->fd = -1;
    fd->events = POLLIN;
    memset(buf, 0, sizeof(buffer));
}

int init_sock(in_port_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return STATUS_ERROR;
    }

    int sockopt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == -1) {
        perror("setsockopt");
        return STATUS_ERROR;
    }

    struct sockaddr_in serveraddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port),
    };

    if (bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind");
        return STATUS_ERROR;
    }

    if (listen(fd, BACKLOG) == -1) {
        perror("listen");
        return STATUS_ERROR;
    }

    printf("Server listening on port %d\n", port);

    return fd;
}

int accept_new(int newfd, struct pollfd *fds) {
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (fds[i].fd == -1) {
            fds[i].fd = newfd;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_ERROR;
}

int start_server(in_port_t port, int dbfd, header_t *header, employee_t *employees) {
    struct pollfd fds[MAX_CLIENTS + 1] = {0};
    buffers buffers = {0};

    for (int i = 1; i < MAX_CLIENTS; i++) {
        reset_state(&fds[i], &buffers[i]);
    }

    int listenfd = init_sock(port);
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;
    int nfds = 1;

    // Poll for network events
    while (1) {
        int nevents = poll(fds, nfds, -1);
        if (nevents == -1) {
            perror("poll");
            return STATUS_ERROR;
        }

        // Handle new connections
        if (fds[0].revents & POLLIN) {
            nevents--;

            struct sockaddr_in addr;
            socklen_t addrlen = sizeof(addr);

            int newfd = accept(listenfd, (struct sockaddr *)&addr, &addrlen);
            if (newfd == -1) {
                perror("accept");
            } else {
                if (accept_new(newfd, fds) == STATUS_ERROR) {
                    printf("Server full: closing connection %s:%d\n", inet_ntoa(addr.sin_addr),
                           ntohs(addr.sin_port));
                    close(newfd);
                } else {
                    printf("New connection %s:%d\n", inet_ntoa(addr.sin_addr),
                           ntohs(addr.sin_port));
                    nfds++;
                }
            }
        }

        // Handle poll events
        for (int i = 1; i <= MAX_CLIENTS && nevents > 0; i++) {
            if (fds[i].revents & POLLIN) {
                nevents--;

                int fd = fds[i].fd;
                ssize_t bytes_read = read(fd, &buffers[i], sizeof(buffer));

                // Handle disconnect
                if (bytes_read <= 0) {
                    struct sockaddr_in addr;
                    socklen_t addrlen = sizeof(addr);
                    getpeername(fd, (struct sockaddr *)&addr, &addrlen);

                    printf("Client disconnected: closing connection %s:%d\n",
                           inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

                    close(fd);
                    reset_state(&fds[i], &buffers[i]);

                    nfds--;
                    continue;
                }

                dbproto_hdr_t *hdr = (dbproto_hdr_t *)buffers[i];
                dbproto_hdr_ntoh(hdr);

                if (hdr->ver != PROTO_VER) {
                    struct sockaddr_in addr;
                    socklen_t addrlen = sizeof(addr);
                    getpeername(fd, (struct sockaddr *)&addr, &addrlen);
                    printf("Incorrect protocol version: closing connection %s:%d\n",
                           inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

                    hdr->ver = PROTO_VER;
                    hdr->type = MSG_ERROR;
                    hdr->len = 2;
                    dbproto_hdr_hton(hdr);

                    dbproto_error_t *err = (dbproto_error_t *)&hdr[1];
                    strncpy(err->msg, "incorrect protocol version", sizeof(err->msg));

                    write(fd, buffers[i], sizeof(buffer));
                    close(fd);
                    reset_state(&fds[i], &buffers[i]);

                    nfds--;
                    continue;
                }

                if (hdr->type == MSG_EMPLOYEE_ADD) {
                    dbproto_employee_t *employee = (dbproto_employee_t *)&hdr[1];
                    add_employee(header, &employees, employee->data);
                    write_file(dbfd, header, employees);

                    hdr->ver = PROTO_VER;
                    hdr->type = MSG_EMPLOYEE_ADD;
                    hdr->len = 1;
                    dbproto_hdr_hton(hdr);

                    write(fd, buffers[i], sizeof(buffer));
                    continue;
                }
            }
        }
    }

    return STATUS_SUCCESS;
}
