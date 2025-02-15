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
           "            [-n '<name>,<address>,<hours>']\n"
           "\n"
           "       -a   server address (required)\n"
           "       -p   server port (required)\n"
           "       -n   new employee\n"
           "\n");
    return;
}

int main(int argc, char *argv[]) {
    int ch;
    in_addr_t addr = 0;
    in_port_t port = 0;
    char *newstr = NULL;

    while ((ch = getopt(argc, argv, "a:p:n:h")) != -1) {
        switch (ch) {
        case 'a':
            addr = inet_addr(optarg);
            break;
        case 'p':
            port = htons(atoi(optarg));
            break;
        case 'n':
            newstr = optarg;
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

    if (newstr != NULL) {
        hdr->ver = PROTO_VER;
        hdr->type = MSG_EMPLOYEE_ADD;
        hdr->len = 2;
        dbproto_hdr_hton(hdr);

        dbproto_employee_t *employee = (dbproto_employee_t *)&hdr[1];
        strncpy(employee->data, newstr, sizeof(dbproto_employee_t));
    }

    write(sock, buf, sizeof(buf));
    read(sock, buf, sizeof(buf));
    dbproto_hdr_ntoh(hdr);

    if (hdr->type == MSG_ERROR) {
        if (hdr->len > 1) {
            dbproto_error_t *err = (dbproto_error_t *)&hdr[1];
            printf("ERROR: %s\n", err->msg);
        } else {
            printf("ERROR: unknown server error\n");
        }
        close(sock);
        return STATUS_ERROR;
    }

    close(sock);
    return STATUS_SUCCESS;
}
