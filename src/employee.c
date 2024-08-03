#include "employee.h"
#include "common.h"
#include "header.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int read_employees(int fd, struct header *header, struct employee **employeesOut) {
    int count = header->count;

    struct employee *employees = calloc(count, sizeof(struct employee));
    if (employees == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    if (lseek(fd, sizeof(struct header), SEEK_SET) == -1) {
        perror("lseek");
        free(employees);
        return STATUS_ERROR;
    }

    size_t size = sizeof(struct employee) * count;
    if (read(fd, employees, size) != size) {
        perror("read");
        free(employees);
        return STATUS_ERROR;
    }

    int i;
    for (i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;

    return STATUS_SUCCESS;
}

int add_employee(struct header *header, struct employee *employees, char *addstring) {
    char *name = strtok(addstring, ",");
    char *address = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");
    int sub = header->count - 1;

    strncpy(employees[sub].name, name, sizeof(employees[sub].name));
    strncpy(employees[sub].address, address, sizeof(employees[sub].address));
    employees[sub].hours = atoi(hours);

    return STATUS_SUCCESS;
}
