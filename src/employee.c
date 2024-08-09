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

void list_employees(int count, struct employee *employees) {
    int i;
    for (i = 0; i < count; i++) {
        printf("name: %s, address: %s, hours: %d\n", employees[i].name, employees[i].address,
               employees[i].hours);
    }
}

int add_employee(struct header *header, struct employee **employees, char *addstring) {
    header->count++;
    struct employee *newEmployees = realloc(*employees, sizeof(struct employee) * header->count);

    char *name = strtok(addstring, ",");
    char *address = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");
    int sub = header->count - 1;

    strncpy(newEmployees[sub].name, name, sizeof(newEmployees[sub].name));
    strncpy(newEmployees[sub].address, address, sizeof(newEmployees[sub].address));
    newEmployees[sub].hours = atoi(hours);

    *employees = newEmployees;

    return STATUS_SUCCESS;
}

int update_employee(struct header *header, struct employee *employees, char *updstring) {
    char *name = strtok(updstring, ",");
    char *address = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    int i;
    for (i = 0; i < header->count; i++) {
        if (strncmp(employees[i].name, name, sizeof(employees[i].name)) == 0) {
            strncpy(employees[i].address, address, sizeof(employees[i].address));
            employees[i].hours = atoi(hours);
        }
    }

    return STATUS_SUCCESS;
}
