#include "employee.h"
#include "file.h"
#include "header.h"
#include "server.h"

#include "../common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_usage() {
    printf("usage: dbsrv [-n] -f <database_file> -p <port>\n"
           "       -n   create new database file\n"
           "       -f   path to database file (required)\n"
           "       -p   port to listen on (required)\n\n");
    return;
}

int main(int argc, char *argv[]) {
    int ch;
    bool newfile = false;
    char *filepath = NULL;
    in_port_t port = 0;

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
            print_usage();
            return STATUS_ERROR;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required arguement\n");
        print_usage();
        return STATUS_ERROR;
    }

    if (port == 0) {
        printf("Valid port is a required arguement\n");
        print_usage();
        return STATUS_ERROR;
    }

    int fd = -1;
    header_t *header = NULL;
    employee_t *employees = NULL;

    if (newfile) {
        fd = open_file(filepath);
        if (fd >= 0) {
            printf("Database file already exists\n");

            if (validate_header(fd, &header) == STATUS_ERROR) {
                printf("Databse file is not valid\n");
                close(fd);
                return STATUS_ERROR;
            }
        } else {
            fd = create_file(filepath);
            if (fd == STATUS_ERROR) {
                printf("Creating database file failed\n");
                return STATUS_ERROR;
            }

            if (create_header(&header) == STATUS_ERROR) {
                printf("Creating database file header failed\n");
                close(fd);
                return STATUS_ERROR;
            }

            if (write_file(fd, header, employees) == STATUS_ERROR) {
                printf("Writing to database file failed\n");
                close(fd);
                return STATUS_ERROR;
            }
        }
    } else {
        fd = open_file(filepath);
        if (fd == STATUS_ERROR) {
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

    if (start_server(port, fd, header, employees) == STATUS_ERROR) {
        printf("Database server failed\n");
        close(fd);
        return STATUS_ERROR;
    }

    close(fd);
    return STATUS_SUCCESS;
}
