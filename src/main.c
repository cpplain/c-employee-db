#include "common.h"
#include "employee.h"
#include "file.h"
#include "header.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_usage(char *argv[]) {
    printf("usage: ./edb -f <database_file> [-n] [-l] [-a '<name>,<address>,<hours>']\n"
           "       -f   path to database file (required)\n"
           "       -n   create new database file\n"
           "       -l   list all employees\n"
           "       -a   add employee to database\n");
    return;
}

int main(int argc, char *argv[]) {
    int ch;
    char *filepath = NULL;
    char *addstring = NULL;
    bool newfile = false;
    bool list = false;

    while ((ch = getopt(argc, argv, "f:nla:")) != -1) {
        switch (ch) {
        case 'f':
            filepath = optarg;
            break;
        case 'n':
            newfile = true;
            break;
        case 'l':
            list = true;
            break;
        case 'a':
            addstring = optarg;
            break;
        case '?':
        default:
            printf("Unknown option\n");
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
            printf("Unable to create database file\n");
            return STATUS_ERROR;
        }

        if (create_header(&header) == STATUS_ERROR) {
            printf("Unable to create database file header\n");
            close(fd);
            return STATUS_ERROR;
        }
    } else {
        if ((fd = open_file(filepath)) == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return STATUS_ERROR;
        }

        if (validate_header(fd, &header) == STATUS_ERROR) {
            printf("Databse file is not valid\n");
            close(fd);
            return STATUS_ERROR;
        }
    }

    if (read_employees(fd, header, &employees) == STATUS_ERROR) {
        printf("Unable to read employees\n");
        close(fd);
        return STATUS_ERROR;
    }

    if (addstring) {
        header->count++;
        employees = realloc(employees, sizeof(struct employee) * header->count);
        if (add_employee(header, employees, addstring) == STATUS_ERROR) {
            printf("Unable to add employee\n");
            close(fd);
            return STATUS_ERROR;
        }
    }

    if (list) {
        list_employees(header->count, employees);
    }

    if (write_file(fd, header, employees) == STATUS_ERROR) {
        printf("Unable to output data to database file\n");
        close(fd);
        return STATUS_ERROR;
    }

    close(fd);
    return STATUS_SUCCESS;
}
