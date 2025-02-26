#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define STATUS_ERROR -1
#define STATUS_SUCCESS 0

#define PROTO_VER 1
#define BUF_SIZE 4096

typedef enum {
    MSG_EMPLOYEE_LIST,
    MSG_EMPLOYEE_ADD,
    MSG_EMPLOYEE_DEL,
    MSG_ERROR,
} dbproto_type_t;

typedef struct {
    uint16_t ver;
    dbproto_type_t type;
    uint16_t len;
} dbproto_hdr_t;

typedef struct {
    char msg[256];
} dbproto_error_t;

typedef struct {
    char data[1024];
} dbproto_employee_add_t;

typedef struct {
    char name[256];
    char address[256];
    unsigned int hours;
} dbproto_employee_list_t;

void dbproto_hdr_hton(dbproto_hdr_t *hdr);

void dbproto_hdr_ntoh(dbproto_hdr_t *hdr);

#endif
