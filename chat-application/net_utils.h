#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <stdint.h>
#include "protocol.h"

int send_msg(int fd, uint8_t type, void *payload, uint32_t len);
int recv_msg(int fd, uint8_t *type, void *buf, uint32_t max_len);

#endif