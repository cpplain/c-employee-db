#include "file.h"
#include "common.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

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

int write_file(int fd, struct header *header, struct employee *employees) {
    int count = header->count;

    header->magic = htonl(header->magic);
    header->version = htons(header->version);
    header->count = htons(header->count);
    header->filesize = htonl(sizeof(struct header) + (sizeof(struct employee) * count));

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        return STATUS_ERROR;
    }

    if (write(fd, header, sizeof(struct header)) == -1) {
        perror("write");
        return STATUS_ERROR;
    }

    int i;
    for (i = 0; i < count; i++) {
        employees[i].hours = htonl(employees[i].hours);
        if (write(fd, &employees[i], sizeof(struct employee)) == -1) {
            perror("write");
            return STATUS_ERROR;
        }
    }

    return STATUS_SUCCESS;
}
