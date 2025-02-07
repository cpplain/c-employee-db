#include "header.h"

#include "../common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int create_header(header_t **headerOut) {
    header_t *header = calloc(1, sizeof(header_t));
    if (header == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = DB_VERSION;
    header->count = 0;
    header->filesize = sizeof(header_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}

int validate_header(int fd, header_t **headerOut) {
    header_t *header = calloc(1, sizeof(header_t));
    if (header == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(header_t)) != sizeof(header_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("Database header magic does not match expected\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != DB_VERSION) {
        printf("Database header version does not match expected\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    if (fstat(fd, &dbstat) == -1) {
        perror("fstat");
        free(header);
        return STATUS_ERROR;
    };

    if (header->filesize != dbstat.st_size) {
        printf("Database file size does not match expected\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}
