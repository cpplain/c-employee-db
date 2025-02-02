#include "employee.h"
#include "file.h"
#include "header.h"

#include "../common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_usage(char *argv[]) {
    printf("usage: ./edb [-n] -f <database_file> [-l]\n"
           "             [-a '<name>,<address>,<hours>']\n"
           "             [-u '<name>,<address>,<hours>']\n"
           "             [-d '<name>']\n"
           "                                            \n"
           "       -f   path to database file (required)\n"
           "       -n   create new database file\n"
           "       -l   list all employees\n"
           "       -a   add employee to database\n"
           "       -u   update employee by name\n"
           "       -d   delete employee by name\n");
    return;
}

int main(int argc, char *argv[]) {
    int ch;
    char *filepath = NULL;
    char *addstring = NULL;
    char *updstring = NULL;
    char *delstring = NULL;
    bool newfile = false;
    bool list = false;

    while ((ch = getopt(argc, argv, "nf:la:u:d:h")) != -1) {
        switch (ch) {
        case 'n':
            newfile = true;
            break;
        case 'f':
            filepath = optarg;
            break;
        case 'l':
            list = true;
            break;
        case 'a':
            addstring = optarg;
            break;
        case 'u':
            updstring = optarg;
            break;
        case 'd':
            delstring = optarg;
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

    int fd = -1;
    struct header *header = NULL;
    struct employee *employees = NULL;

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

    if (addstring) {
        add_employee(header, &employees, addstring);
    }

    if (updstring) {
        if (update_employee(header, employees, updstring) == STATUS_ERROR) {
            printf("Updating employee failed\n");
            close(fd);
            return STATUS_ERROR;
        }
    }

    if (delstring) {
        if (delete_employee(header, employees, delstring) == STATUS_ERROR) {
            printf("Deleting employee failed\n");
            close(fd);
            return STATUS_ERROR;
        }
    }

    if (list) {
        list_employees(header->count, employees);
    }

    if (write_file(fd, header, employees) == STATUS_ERROR) {
        printf("Writing to database file failed\n");
        close(fd);
        return STATUS_ERROR;
    }

    close(fd);
    return STATUS_SUCCESS;
}
