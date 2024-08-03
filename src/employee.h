#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "header.h"

struct employee {
    char name[256];
    char address[256];
    unsigned int hours;
};

int read_employees(int fd, struct header *header, struct employee **employeesOut);
int add_employee(struct header *header, struct employee *employees, char *addstring);

#endif
