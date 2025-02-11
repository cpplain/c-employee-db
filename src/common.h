#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define STATUS_ERROR -1
#define STATUS_SUCCESS 0

#define PROTO_VER 1

typedef enum {
    MSG_PROTO_VER,
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

#endif
