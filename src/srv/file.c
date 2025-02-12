#include "file.h"

#include "../common.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int create_file(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd >= 0) {
        close(fd);
        printf("Database file already exists\n");
        return STATUS_ERROR;
    }

    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return STATUS_ERROR;
    }

    return fd;
}

int open_file(char *filename) {
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        perror("open");
        return STATUS_ERROR;
    }

    return fd;
}

int write_file(int fd, header_t *header, employee_t *employees) {
    int count = header->count;
    int filesize = sizeof(header_t) + (sizeof(employee_t) * count);

    header->magic = htonl(header->magic);
    header->version = htons(header->version);
    header->count = htons(count);
    header->filesize = htonl(filesize);

    if (ftruncate(fd, filesize) < 0) {
        perror("ftruncate");
        return STATUS_ERROR;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        return STATUS_ERROR;
    }

    if (write(fd, header, sizeof(header_t)) < 0) {
        perror("write");
        return STATUS_ERROR;
    }

    int i;
    for (i = 0; i < count; i++) {
        employees[i].hours = htonl(employees[i].hours);
        if (write(fd, &employees[i], sizeof(employee_t)) < 0) {
            perror("write");
            return STATUS_ERROR;
        }
    }

    return STATUS_SUCCESS;
}
