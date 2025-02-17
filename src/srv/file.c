#include "file.h"

#include "../common.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int create_file(char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
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
    header->count = htons(header->count);
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

    for (int i = 0; i < count; i++) {
        employees[i].hours = htonl(employees[i].hours);
        if (write(fd, &employees[i], sizeof(employee_t)) < 0) {
            perror("write");
            return STATUS_ERROR;
        }
    }

    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(filesize);

    for (int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    return STATUS_SUCCESS;
}
