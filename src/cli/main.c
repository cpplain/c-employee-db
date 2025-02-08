#include "../common.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
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

    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return STATUS_ERROR;
    };

    struct sockaddr_in serveraddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = addr,
        .sin_port = port,
    };

    if (connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("connect");
        close(fd);
        return STATUS_ERROR;
    };

    return STATUS_SUCCESS;
}
