#include "common.h"
#include "file.h"

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

void print_usage(char *argv[]) {
    printf("Usage: %s [-n] -f <database_file>\n", argv[0]);
    printf("    -n    create new database file\n");
    printf("    -f    path to database file (required)\n");
    return;
}

int main(int argc, char *argv[]) {
    int ch;
    bool newfile = false;
    char *filepath = NULL;

    while ((ch = getopt(argc, argv, "nf:")) != -1) {
        switch (ch) {
        case 'n':
            newfile = true;
            break;
        case 'f':
            filepath = optarg;
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
    if (newfile) {
        if ((fd = create_file(filepath)) == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return STATUS_ERROR;
        }
    } else {
        if ((fd = open_file(filepath)) == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return STATUS_ERROR;
        }
    }

    close(fd);
    return STATUS_SUCCESS;
}
