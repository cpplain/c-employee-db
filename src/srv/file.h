#ifndef FILE_H
#define FILE_H

#include "employee.h"
#include "header.h"

int create_file(char *filename);
int open_file(char *filename);
int write_file(int fd, struct header *header, struct employee *employees);

#endif
