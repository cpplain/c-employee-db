#include "employee.h"
#include "header.h"

#include "../common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int read_employees(int fd, header_t *header, employee_t **employeesOut) {
    int count = header->count;

    employee_t *employees = calloc(count, sizeof(employee_t));
    if (employees == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    if (lseek(fd, sizeof(header_t), SEEK_SET) == -1) {
        perror("lseek");
        free(employees);
        return STATUS_ERROR;
    }

    size_t size = sizeof(employee_t) * count;
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

void list_employees(int count, employee_t *employees) {
    int i;
    for (i = 0; i < count; i++) {
        int num = i + 1;
        printf("Employee %d\n"
               "  Name: %s\n"
               "  Address: %s\n"
               "  Hours: %d\n",
               num, employees[i].name, employees[i].address, employees[i].hours);
    }
}

void add_employee(header_t *header, employee_t **employees, char *addstring) {
    header->count++;
    employee_t *newEmployees = realloc(*employees, sizeof(employee_t) * header->count);

    char *name = strtok(addstring, ",");
    char *address = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");
    int sub = header->count - 1;

    strncpy(newEmployees[sub].name, name, sizeof(newEmployees[sub].name));
    strncpy(newEmployees[sub].address, address, sizeof(newEmployees[sub].address));
    newEmployees[sub].hours = atoi(hours);

    *employees = newEmployees;
}

int update_employee(header_t *header, employee_t *employees, char *updstring) {
    char *name = strtok(updstring, ",");
    char *address = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    int i;
    for (i = 0; i < header->count; i++) {
        if (strncmp(employees[i].name, name, sizeof(employees[i].name)) == 0) {
            strncpy(employees[i].address, address, sizeof(employees[i].address));
            employees[i].hours = atoi(hours);
            return STATUS_SUCCESS;
        }
    }

    return STATUS_ERROR;
}

int delete_employee(header_t *header, employee_t *employees, char *delstring) {
    int i;
    int count = header->count;
    bool shift = false;
    int status = STATUS_ERROR;

    for (i = 0; i < count; i++) {
        if (shift) {
            strncpy(employees[i - 1].name, employees[i].name, sizeof(employees[i - 1].name));
            strncpy(employees[i - 1].address, employees[i].address,
                    sizeof(employees[i - 1].address));
            employees[i - 1].hours = employees[i].hours;
        }

        if (strncmp(employees[i].name, delstring, sizeof(employees[i].name)) == 0) {
            header->count--;
            shift = true;
            status = STATUS_SUCCESS;
        }
    }

    return status;
}
