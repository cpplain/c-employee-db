#include "common.h"

#include <arpa/inet.h>

void dbproto_hdr_hton(dbproto_hdr_t *hdr) {
    hdr->ver = htons(hdr->ver);
    hdr->type = htons(hdr->type);
    hdr->len = htons(hdr->len);
}

void dbproto_hdr_ntoh(dbproto_hdr_t *hdr) {
    hdr->ver = ntohs(hdr->ver);
    hdr->type = ntohs(hdr->type);
    hdr->len = ntohs(hdr->len);
}
