#ifndef SERVER_H
#define SERVER_H

#include "employee.h"
#include "header.h"

#include <netinet/in.h>

int start_server(in_port_t port, header_t *header, employee_t *employees);

#endif
