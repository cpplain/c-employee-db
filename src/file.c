#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "file.h"
#include "common.h"

int create_file(char *filename) {
    int fd = -1;
    if ((fd = open(filename, O_RDONLY)) != -1) {
        close(fd);
        printf("Database file already exists\n");
        return STATUS_ERROR;
    }

    if ((fd = open(filename, O_RDWR | O_CREAT, 0644)) == -1) {
        perror("open");
        return STATUS_ERROR;
    }

    return fd;
}

int open_file(char *filename) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        return STATUS_ERROR;
    }
    
    return fd;
}
