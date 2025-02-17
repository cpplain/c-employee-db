#include "../common.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void print_usage() {
    printf("usage: dbcli -a <address> -p <port> [-l]\n"
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
    int list = 0;

    while ((ch = getopt(argc, argv, "a:p:n:lh")) != -1) {
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
        case 'l':
            list = 1;
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
        hdr->len = 1;
        dbproto_hdr_hton(hdr);

        dbproto_employee_add_t *employee = (dbproto_employee_add_t *)&hdr[1];
        strncpy(employee->data, newstr, sizeof(dbproto_employee_add_t));
    }

    if (list == 1) {
        hdr->ver = PROTO_VER;
        hdr->type = MSG_EMPLOYEE_LIST;
        hdr->len = 0;
        dbproto_hdr_hton(hdr);
    }

    write(sock, buf, sizeof(buf));
    read(sock, buf, sizeof(buf));
    dbproto_hdr_ntoh(hdr);

    if (hdr->type == MSG_ERROR) {
        if (hdr->len > 0) {
            dbproto_error_t *err = (dbproto_error_t *)&hdr[1];
            printf("ERROR: %s\n", err->msg);
        } else {
            printf("ERROR: unknown server error\n");
        }
        close(sock);
        return STATUS_ERROR;
    }

    if (hdr->type == MSG_EMPLOYEE_LIST) {
        dbproto_employee_list_t *list = (dbproto_employee_list_t *)&hdr[1];
        printf("Employees:\n"
               "    No. Name, Address, Hours\n"
               "    ------------------------\n");
        for (int i = 0; i < hdr->len; i++) {
            printf("    %d. %s, %s, %d\n", i + 1, list[i].name, list[i].address,
                   ntohs(list[i].hours));
        }
        printf("\n");
    }

    close(sock);
    return STATUS_SUCCESS;
}
