#include "server.h"

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

int init_server_sock(unsigned int port) {
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

int free_slot(struct pollfd fds[]) {
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (fds[i].fd == -1) {
            return i;
        }
    }
    return STATUS_ERROR;
}

int start_server(unsigned int port) {
    struct pollfd fds[MAX_CLIENTS + 1] = {0};
    buffers buffers = {0};

    for (int i = 1; i < MAX_CLIENTS; i++) {
        reset_state(&fds[i], &buffers[i]);
    }

    int listenfd = init_server_sock(port);
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
            struct sockaddr_in clientaddr;
            socklen_t addrlen = sizeof(clientaddr);

            // Assign client to pollfds
            int newfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);
            if (newfd == -1) {
                perror("accept");
            } else {
                int slot = free_slot(fds);
                if (slot < 0) {
                    printf("Server full: closing connection %s:%d\n",
                           inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                    close(newfd);
                } else {
                    printf("New connection %s:%d\n", inet_ntoa(clientaddr.sin_addr),
                           ntohs(clientaddr.sin_port));
                    fds[slot].fd = newfd;

                    nfds++;
                }
            }

            nevents--;
        }

        // Check pollfds for events
        for (int i = 1; i <= MAX_CLIENTS && nevents > 0; i++) {
            if (fds[i].revents & POLLIN) {
                int fd = fds[i].fd;
                ssize_t bytes_read = read(fd, buffers[i], sizeof(buffer));

                struct sockaddr_in clientaddr;
                socklen_t addrlen = sizeof(clientaddr);
                getpeername(fd, (struct sockaddr *)&clientaddr, &addrlen);

                // Handle client request
                if (bytes_read <= 0) {
                    printf("Client disconnected: closing connection %s:%d\n",
                           inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                    close(fd);
                    reset_state(&fds[i], &buffers[i]);

                    nfds--;
                } else {
                    dbproto_hdr_t *hdr = (dbproto_hdr_t *)buffers[i];

                    if (hdr->ver != PROTO_VER) {
                        printf("Incorrect protocol version: closing connection %s:%d\n",
                               inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                        hdr->ver = PROTO_VER;
                        hdr->type = MSG_ERROR;
                        hdr->len = 2;

                        dbproto_error_t *err = (dbproto_error_t *)&hdr[1];
                        strncpy(err->msg, "incorrect protocol version", sizeof(err->msg));

                        write(fd, buffers[i], sizeof(buffer));
                        close(fd);
                        reset_state(&fds[i], &buffers[i]);

                        nfds--;
                    } else {
                        switch (hdr->type) {
                        case MSG_EMPLOYEE_ADD:
                            hdr->ver = PROTO_VER;
                            hdr->type = MSG_EMPLOYEE_ADD;
                            hdr->len = 2;

                            // TODO: return status

                            write(fd, buffers[i], sizeof(buffer));
                            break;
                        default:
                            break;
                        }
                    }
                }

                nevents--;
            }
        }
    }

    return STATUS_SUCCESS;
}
