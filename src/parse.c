#include "parse.h"
#include "common.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int create_header(struct header **headerOut) {
    struct header *header = calloc(1, sizeof(struct header));
    if (header == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = DB_VERSION;
    header->count = 0;
    header->filesize = sizeof(struct header);

    *headerOut = header;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct header *header) {
    header->magic = htonl(header->magic);
    header->version = htons(header->version);
    header->count = htons(header->count);
    header->filesize = htonl(sizeof(struct header));

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        return STATUS_ERROR;
    }

    if (write(fd, header, sizeof(struct header)) == -1) {
        perror("write");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

int validate_header(int fd, struct header **headerOut) {
    struct header *header = calloc(1, sizeof(struct header));
    if (header == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct header)) != sizeof(struct header)) {
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
