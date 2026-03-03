#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include "protocol.h"
#include "net_utils.h"

static int read_all(int fd, void *buf, int n) {
    int total = 0;
    char *p = buf;
    while (total < n) {
        int r = read(fd, p + total, n - total);
        if (r <= 0) return -1;
        total += r;
    }
    return total;
}

static int write_all(int fd, void *buf, int n) {
    int total = 0;
    char *p = buf;
    while (total < n) {
        int w = write(fd, p + total, n - total);
        if (w <= 0) return -1;
        total += w;
    }
    return total;
}

int send_msg(int fd, uint8_t type, void *payload, uint32_t len) {
    msg_header_t hdr;
    hdr.type   = type;
    hdr.length = htonl(len);
    if (write_all(fd, &hdr, sizeof(hdr)) < 0) return -1;
    if (len > 0 && write_all(fd, payload, len) < 0) return -1;
    return 0;
}

int recv_msg(int fd, uint8_t *type, void *buf, uint32_t max_len) {
    msg_header_t hdr;
    if (read_all(fd, &hdr, sizeof(hdr)) < 0) return -1;
    *type = hdr.type;
    uint32_t len = ntohl(hdr.length);
    if (len == 0) return 0;
    if (len > max_len) return -1;
    if (read_all(fd, buf, len) < 0) return -1;
    return len;
}