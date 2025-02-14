#include "../common.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void print_usage() {
    printf("usage: dbcli -a <address> -p <port>\n"
           "       -a   server address (required)\n"
           "       -p   server port (required)\n\n");
    return;
}

int main(int argc, char *argv[]) {
    int ch;
    in_addr_t addr = 0;
    in_port_t port = 0;

    while ((ch = getopt(argc, argv, "a:p:h")) != -1) {
        switch (ch) {
        case 'a':
            addr = inet_addr(optarg);
            break;
        case 'p':
            port = htons(atoi(optarg));
            break;
        case 'h' | '?':
        default:
            print_usage();
            return STATUS_ERROR;
        }
    }

    if (addr == 0) {
        printf("Server address is a required arguement\n");
        print_usage();
        return STATUS_ERROR;
    }

    if (port == 0) {
        printf("Server port is a required arguement\n");
        print_usage();
        return STATUS_ERROR;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return STATUS_ERROR;
    };

    struct sockaddr_in serveraddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = addr,
        .sin_port = port,
    };

    if (connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("connect");
        close(sock);
        return STATUS_ERROR;
    };

    char buf[4096] = {0};
    dbproto_hdr_t *hdr = (dbproto_hdr_t *)buf;
    hdr->ver = PROTO_VER;
    hdr->type = MSG_EMPLOYEE_ADD;
    hdr->len = 2;

    // TODO: send employee properties

    write(sock, buf, sizeof(buf));
    read(sock, buf, sizeof(buf));

    switch (hdr->type) {
    case MSG_ERROR:
        if (hdr->len > 1) {
            dbproto_error_t *err = (dbproto_error_t *)&hdr[1];
            printf("ERROR: %s\n", err->msg);
        } else {
            printf("ERROR: unknown server error\n");
        }
        close(sock);
        return STATUS_ERROR;
    default:
        break;
    }

    close(sock);
    return STATUS_SUCCESS;
}
