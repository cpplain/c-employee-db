#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "header.h"

typedef struct {
    char name[256];
    char address[256];
    unsigned int hours;
} employee_t;

int read_employees(int fd, header_t *header, employee_t **employeesOut);
void list_employees(int count, employee_t *employees);
void add_employee(header_t *header, employee_t **employees, char *addstring);
int update_employee(header_t *header, employee_t *employees, char *updstring);
int delete_employee(header_t *header, employee_t *employees, char *delstring);

#endif
