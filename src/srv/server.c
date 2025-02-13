#include "server.h"

#include "../common.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 16
#define PORT 8080
#define BUFF_SIZE 4096
#define BACKLOG 8

typedef enum {
    STATE_DISCONNECTED,
    STATE_CONNECTED,
} connection_state_t;

typedef struct {
    int fd;
    connection_state_t state;
    char buffer[BUFF_SIZE];
    char addr[18];
    unsigned short port;
} client_state_t;

int setup_server_sock(unsigned int port) {
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

void unset_client_structs(struct pollfd fds[], client_state_t clients[], int slot) {
    fds[slot].fd = -1;
    fds[slot].events = POLLIN;

    slot--;
    clients[slot].fd = -1;
    clients[slot].state = STATE_DISCONNECTED;
    clients[slot].port = 0;
    strncpy(clients[slot].addr, "\0", sizeof(clients[slot].addr));
}

int start_server(unsigned int port) {
    struct pollfd fds[MAX_CLIENTS + 1] = {0};
    client_state_t clients[MAX_CLIENTS] = {0};

    for (int i = 1; i < MAX_CLIENTS; i++) {
        unset_client_structs(fds, clients, i);
    }

    int listenfd = setup_server_sock(port);
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

            struct sockaddr_in clientaddr;
            socklen_t addrlen = sizeof(clientaddr);

            int newfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);
            if (newfd == -1) {
                perror("accept");
            } else {
                // Assign client to slot in fds and clients
                int slot = -1;
                for (int i = 1; i <= MAX_CLIENTS; i++) {
                    if (fds[i].fd == -1) {
                        slot = i;
                        break;
                    }
                }

                if (slot < 0) {
                    printf("Server full: closing connection %s:%d\n",
                           inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                    close(newfd);
                } else {
                    fds[slot].fd = newfd;

                    slot--;
                    clients[slot].fd = newfd;
                    clients[slot].state = STATE_CONNECTED;
                    clients[slot].port = ntohs(clientaddr.sin_port);
                    strncpy(clients[slot].addr, inet_ntoa(clientaddr.sin_addr),
                            sizeof(clients[slot].addr));

                    printf("Accepted connection %s:%d\n", clients[slot].addr, clients[slot].port);

                    nfds++;
                }
            }
        }

        // Check pollfds for events
        for (int i = 1; i <= MAX_CLIENTS && nevents > 0; i++) {
            if (fds[i].revents & POLLIN) {
                nevents--;

                int fd = fds[i].fd;
                int j = i - 1;
                ssize_t bytes_read = read(fd, clients[j].buffer, sizeof(clients[j].buffer));

                // Handle client disconnection or connection error
                if (bytes_read <= 0) {
                    printf("Client disconnected: closing connection %s:%d\n", clients[j].addr,
                           clients[j].port);

                    close(fd);
                    unset_client_structs(fds, clients, i);

                    nfds--;
                    continue;
                }

                // Handle client request
                dbproto_hdr_t *hdr = (dbproto_hdr_t *)clients[j].buffer;

                // Handle incorrect protocol version from client
                if (hdr->ver != PROTO_VER) {
                    printf("Incorrect protocol version: closing connection %s:%d\n",
                           clients[j].addr, clients[j].port);

                    hdr->ver = 1;
                    hdr->type = MSG_ERROR;
                    hdr->len = 2;

                    dbproto_error_t *err = (dbproto_error_t *)&hdr[1];
                    strncpy(err->msg, "incorrect protocol version", sizeof(err->msg));

                    write(fd, clients[j].buffer, sizeof(clients[j].buffer));
                    close(fd);
                    unset_client_structs(fds, clients, i);

                    nfds--;
                    continue;
                }

                // Handle protocol request from client
                if (hdr->type == MSG_PROTO_VER) {
                    hdr->ver = 1;
                    hdr->type = MSG_PROTO_VER;
                    hdr->len = 1;

                    write(fd, clients[j].buffer, sizeof(clients[j].buffer));
                    continue;
                }
            }
        }
    }

    return STATUS_SUCCESS;
}
