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
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED,
    STATE_HELLO,
    STATE_MSG,
    STATE_GOODBYE,
} connection_state_t;

typedef struct {
    int fd;
    connection_state_t state;
    char buffer[BUFF_SIZE];
    char *addr;
    unsigned short port;
} client_state_t;

char empty_str[] = {"\0"};

int find_free_slot(client_state_t *states) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (states[i].fd == -1) {
            return i;
        }
    }
    return STATUS_ERROR;
}

int find_slot_by_fd(client_state_t *states, int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (states[i].fd == fd) {
            return i;
        }
    }
    return STATUS_ERROR;
}

int run_server(unsigned int port) {
    struct pollfd fds[MAX_CLIENTS + 1] = {0};
    client_state_t states[MAX_CLIENTS] = {0};
    for (int i = 0; i < MAX_CLIENTS; i++) {
        fds[i + 1].fd = -1;
        fds[i + 1].events = POLLIN;
        states[i].fd = -1;
        states[i].state = STATE_NEW;
        states[i].addr = empty_str;
        states[i].port = 0;
    }

    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        return STATUS_ERROR;
    }

    int sockopt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == -1) {
        perror("setsockopt");
        return STATUS_ERROR;
    }

    struct sockaddr_in serveraddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port),
    };

    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind");
        return STATUS_ERROR;
    }

    if (listen(listenfd, BACKLOG) == -1) {
        perror("listen");
        return STATUS_ERROR;
    }

    printf("Server listening on port %d\n", port);

    fds[0].fd = listenfd;
    fds[0].events = POLLIN;
    int nfds = 1;

    while (1) {
        int ret;
        if ((ret = poll(fds, nfds, -1) == -1)) {
            perror("poll");
            return STATUS_ERROR;
        }

        // Handle new connections
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in clientaddr;
            socklen_t addrlen = sizeof(clientaddr);

            int connfd;
            if ((connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen)) == -1) {
                perror("accept");
                continue;
            }

            int slot;
            if ((slot = find_free_slot(states)) == STATUS_ERROR) {
                printf("Server full: closing new connection from %s:%d\n",
                       inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                close(connfd);
            } else {
                states[slot].fd = connfd;
                states[slot].state = STATE_HELLO;
                states[slot].addr = inet_ntoa(clientaddr.sin_addr);
                states[slot].port = ntohs(clientaddr.sin_port);

                printf("Accepted connection from %s:%d\n", states[slot].addr, states[slot].port);

                nfds++;
            }

            ret--;
        }

        // Check clients for events
        for (int i = 1; i <= nfds && ret > 0; i++) {
            if (fds[i].revents & POLLIN) {
                int fd = fds[i].fd;
                int slot = find_slot_by_fd(states, fd);
                ssize_t bytes_read = read(fd, &states[slot].buffer, sizeof(states[slot].buffer));

                if (bytes_read <= 0) {
                    close(fd);

                    if (slot != -1) {
                        printf("Closed connection from %s:%d\n", states[slot].addr,
                               states[slot].port);

                        states[slot].fd = -1;
                        states[slot].state = STATE_DISCONNECTED;
                        states[slot].addr = empty_str;
                        states[slot].port = 0;

                        nfds--;
                    }
                } else {
                    printf("Handling the client connection");
                }

                ret--;
            }
        }
    }

    return STATUS_SUCCESS;
}
