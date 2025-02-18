#include "server.h"
#include "employee.h"
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

void handle_new_connection(int listenfd, struct pollfd *fds, int *nfds) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    int newfd = accept(listenfd, (struct sockaddr *)&addr, &addrlen);
    if (newfd == -1) {
        perror("accept");
        return;
    }

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (fds[i].fd == -1) {
            printf("New connection %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            fds[i].fd = newfd;
            *nfds += 1;
            return;
        }
    }

    printf("Server full: closing connection %s:%d\n", inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port));
    close(newfd);
}

void handle_disconnect(struct pollfd *fd, buffer *buf, int *nfds) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    getpeername(fd->fd, (struct sockaddr *)&addr, &addrlen);

    printf("Client disconnected: closing connection %s:%d\n", inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port));

    close(fd->fd);
    reset_state(fd, buf);

    *nfds -= 1;
}

void handle_proto_mismatch(struct pollfd *fd, buffer *buf, dbproto_hdr_t *hdr, int *nfds) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    getpeername(fd->fd, (struct sockaddr *)&addr, &addrlen);
    printf("Incorrect protocol version: closing connection %s:%d\n", inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port));

    hdr->ver = PROTO_VER;
    hdr->type = MSG_ERROR;
    hdr->len = 1;
    dbproto_hdr_hton(hdr);

    dbproto_error_t *err = (dbproto_error_t *)&hdr[1];
    strncpy(err->msg, "incorrect protocol version", sizeof(err->msg));

    write(fd->fd, buf, sizeof(buffer));
    close(fd->fd);
    reset_state(fd, buf);

    *nfds -= 1;
}

void handle_add_employee(int fd, buffer *buf, dbproto_hdr_t *hdr, header_t *dbhdr,
                         employee_t **employees, int dbfd) {
    dbproto_employee_add_t *employee = (dbproto_employee_add_t *)&hdr[1];
    add_employee(dbhdr, employees, employee->data);
    write_file(dbfd, dbhdr, *employees);

    hdr->ver = PROTO_VER;
    hdr->type = MSG_EMPLOYEE_ADD;
    hdr->len = 0;
    dbproto_hdr_hton(hdr);

    write(fd, buf, sizeof(buffer));
}

void handle_list_employees(int fd, buffer *buf, dbproto_hdr_t *hdr, header_t *dbhdr,
                           employee_t *employees) {
    hdr->ver = PROTO_VER;
    hdr->type = MSG_EMPLOYEE_LIST;
    hdr->len = dbhdr->count;
    dbproto_hdr_hton(hdr);

    dbproto_employee_list_t *list = (dbproto_employee_list_t *)&hdr[1];

    for (int i = 0; i < dbhdr->count; i++) {
        strncpy(list[i].name, employees[i].name, sizeof(employees[i].name));
        strncpy(list[i].address, employees[i].address, sizeof(employees[i].address));
        list[i].hours = htons(employees[i].hours);
    }

    write(fd, buf, sizeof(buffer));
}

int start_server(in_port_t port, int dbfd, header_t *dbhdr, employee_t *employees) {
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
            handle_new_connection(listenfd, fds, &nfds);
        }

        // Handle poll events
        for (int i = 1; i <= MAX_CLIENTS && nevents > 0; i++) {
            if (fds[i].revents & POLLIN) {
                nevents--;

                int fd = fds[i].fd;
                ssize_t bytes_read = read(fd, &buffers[i], sizeof(buffer));

                if (bytes_read <= 0) {
                    handle_disconnect(&fds[i], &buffers[i], &nfds);
                    continue;
                }

                dbproto_hdr_t *hdr = (dbproto_hdr_t *)buffers[i];
                dbproto_hdr_ntoh(hdr);

                if (hdr->ver != PROTO_VER) {
                    handle_proto_mismatch(&fds[i], &buffers[i], hdr, &nfds);
                    continue;
                }

                if (hdr->type == MSG_EMPLOYEE_ADD) {
                    handle_add_employee(fd, &buffers[i], hdr, dbhdr, &employees, dbfd);
                    continue;
                }

                if (hdr->type == MSG_EMPLOYEE_LIST) {
                    handle_list_employees(fd, &buffers[i], hdr, dbhdr, employees);
                    continue;
                }
            }
        }
    }

    return STATUS_SUCCESS;
}
