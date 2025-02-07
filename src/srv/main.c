#include "employee.h"
#include "file.h"
#include "header.h"

#include "../common.h"

#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 256
#define BACKLOG 16

int start_server(unsigned int port) {
    int listenfd;
    if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        return -1;
    }

    int sockopt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == -1) {
        perror("setsockopt");
        return -1;
    }

    struct sockaddr_in serveraddr = {0};
    // memset(&addr, 0, sizeof(addr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind");
        return -1;
    }

    if (listen(listenfd, BACKLOG) == -1) {
        perror("listen");
        return -1;
    }

    printf("Server listening on port %d\n", port);

    struct pollfd fds[MAX_CLIENTS + 1];
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;

    int nfds = 1;

    while (1) {
        int ret;
        if ((ret = poll(fds, nfds, -1) == -1)) {
            perror("poll");
            return -1;
        }

        // Handle new connections
        if (fds[0].events & POLLIN) {
            int connfd;
            struct sockaddr_in clientaddr = {0};
            socklen_t addrlen = sizeof(clientaddr);

            if ((connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen)) == -1) {
                perror("accept");
                continue;
            }

            close(connfd);
        }
    }
}

void print_usage(char *argv[]) {
    printf("usage: dbsrv [-n] -f <database_file> -p <port>\n"
           "\n"
           "       -n   create new database file\n"
           "       -f   path to database file (required)\n"
           "       -p   port to listen on (required)\n");
    return;
}

int main(int argc, char *argv[]) {
    int ch;
    bool newfile = false;
    char *filepath = NULL;
    unsigned short port = 0;

    while ((ch = getopt(argc, argv, "nf:p:h")) != -1) {
        switch (ch) {
        case 'n':
            newfile = true;
            break;
        case 'f':
            filepath = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'h' | '?':
        default:
            print_usage(argv);
            return STATUS_ERROR;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required arguement\n");
        print_usage(argv);
        return STATUS_ERROR;
    }

    if (port == 0) {
        printf("Valid port is a required arguement\n");
        print_usage(argv);
        return STATUS_ERROR;
    }

    int fd = -1;
    header_t *header = NULL;
    employee_t *employees = NULL;

    if (newfile) {
        if ((fd = create_file(filepath)) == STATUS_ERROR) {
            printf("Creating database file failed\n");
            return STATUS_ERROR;
        }

        if (create_header(&header) == STATUS_ERROR) {
            printf("Creating database file header failed\n");
            close(fd);
            return STATUS_ERROR;
        }
    } else {
        if ((fd = open_file(filepath)) == STATUS_ERROR) {
            printf("Opening database file failed\n");
            return STATUS_ERROR;
        }

        if (validate_header(fd, &header) == STATUS_ERROR) {
            printf("Databse file is not valid\n");
            close(fd);
            return STATUS_ERROR;
        }
    }

    if (read_employees(fd, header, &employees) == STATUS_ERROR) {
        printf("Reading employees failed\n");
        close(fd);
        return STATUS_ERROR;
    }

    if (start_server(port) == STATUS_ERROR) {
        printf("Database server failed\n");
        close(fd);
        return STATUS_ERROR;
    }

    if (write_file(fd, header, employees) == STATUS_ERROR) {
        printf("Writing to database file failed\n");
        close(fd);
        return STATUS_ERROR;
    }

    close(fd);
    return STATUS_SUCCESS;
}
