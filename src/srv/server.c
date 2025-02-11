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

int run_server(unsigned int port) {
    // Initialize poll and client state structs
    struct pollfd fds[MAX_CLIENTS + 1] = {0};
    client_state_t clients[MAX_CLIENTS] = {0};

    for (int i = 1; i < MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;

        int j = i - 1;
        clients[j].fd = -1;
        clients[j].state = STATE_DISCONNECTED;
    }

    // Setup socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
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

    // Handle network requests
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

            int newfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);
            if (newfd == -1) {
                perror("accept");
                continue;
            }

            // Assign client to slot in fds and clients
            int slot = -1;
            for (int i = 1; i <= MAX_CLIENTS; i++) {
                if (fds[i].fd == -1) {
                    slot = i;
                    break;
                }
            }
            if (slot < 0) {
                printf("Server full: closing new connection from %s:%d\n",
                       inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                close(newfd);
            } else {
                fds[slot].fd = newfd;

                int c_slot = slot - 1;
                clients[c_slot].fd = newfd;
                clients[c_slot].state = STATE_CONNECTED;
                clients[c_slot].port = ntohs(clientaddr.sin_port);
                strncpy(clients[c_slot].addr, inet_ntoa(clientaddr.sin_addr),
                        sizeof(clients[c_slot].addr));

                printf("Accepted connection from %s:%d\n", clients[c_slot].addr,
                       clients[c_slot].port);

                nfds++;
            }

            nevents--;
        }

        // Check fds for events
        for (int i = 1; i <= MAX_CLIENTS && nevents > 0; i++) {
            if (fds[i].revents & POLLIN) {
                int fd = fds[i].fd;

                int j = i - 1;
                ssize_t bytes_read = read(fd, clients[j].buffer, sizeof(clients[j].buffer));

                // Handle client disconnection or connection error
                if (bytes_read <= 0) {
                    close(fd);
                    printf("Closed connection from %s:%d\n", clients[j].addr, clients[j].port);

                    fds[i].fd = -1;

                    clients[j].fd = -1;
                    clients[j].state = STATE_DISCONNECTED;
                    clients[j].port = 0;
                    strncpy(clients[j].addr, "\0", sizeof(clients[j].addr));

                    nfds--;
                    nevents--;
                    continue;
                }

                // Handle client request
                dbproto_hdr_t *hdr = (dbproto_hdr_t *)clients[j].buffer;

                if (clients[j].state == STATE_CONNECTED) {
                    if (hdr->ver != PROTO_VER) {
                        // TODO: Send error

                        close(fd);
                        printf("Closed connection from %s:%d\n", clients[j].addr, clients[j].port);

                        fds[i].fd = -1;

                        clients[j].fd = -1;
                        clients[j].state = STATE_DISCONNECTED;
                        clients[j].port = 0;
                        strncpy(clients[j].addr, "\0", sizeof(clients[j].addr));

                        nfds--;
                    } else if (hdr->type == MSG_PROTO_VER) {
                        printf("Version sent by client: %d\n", hdr->ver);
                        // TODO: Send success
                    }
                }

                nevents--;
            }
        }
    }

    return STATUS_SUCCESS;
}
