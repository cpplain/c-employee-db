#ifndef HEADER_H
#define HEADER_H

#define HEADER_MAGIC 0x4c4c4144
#define DB_VERSION 0x1

typedef struct {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
} header_t;

int create_header(header_t **headerOut);
int validate_header(int fd, header_t **headerOut);

#endif
