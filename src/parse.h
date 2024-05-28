#ifndef PARSE_H
#define PARSE_H

#define HEADER_MAGIC 0x4c4c4144
#define DB_VERSION 0x1

struct header {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
};

int create_header(struct header **headerOut);
int output_file(int fd, struct header *header);
int validate_header(int fd, struct header **headerOut);

#endif
