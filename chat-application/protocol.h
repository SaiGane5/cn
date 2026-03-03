#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define DISCOVERY_PORT 9090
#define CHAT_PORT      8080
#define BUFFER_SIZE    1024
#define MAX_USERS      64
#define MAX_NAME       32
#define MAX_PASS       32

typedef enum {
    MSG_REGISTER   = 1,
    MSG_LOGIN      = 2,
    MSG_BROADCAST  = 3,
    MSG_PRIVATE    = 4,
    MSG_LIST_USERS = 5,
    MSG_DISCONNECT = 6,
    MSG_AUTH_OK    = 7,
    MSG_AUTH_FAIL  = 8,
    MSG_USER_LIST  = 9,
    MSG_ERROR      = 10,
    MSG_NOTIFY     = 11,
    MSG_STATUS     = 12,
    MSG_HISTORY_REQ= 13,
    MSG_HISTORY_RES= 14
} msg_type_t;

typedef struct {
    uint8_t  type;
    uint32_t length;
} __attribute__((packed)) msg_header_t;

typedef struct {
    char     username[MAX_NAME];
    char     password[MAX_PASS];
    uint16_t port;
} reg_payload_t;

typedef struct {
    char username[MAX_NAME];
    char password[MAX_PASS];
} login_payload_t;

typedef struct {
    char sender[MAX_NAME];
    char text[BUFFER_SIZE];
} broadcast_payload_t;

typedef struct {
    char sender[MAX_NAME];
    char recipient[MAX_NAME];
    char text[BUFFER_SIZE];
    long long timestamp;
} private_payload_t;

typedef struct {
    char username[MAX_NAME];
    char status[16];
} status_payload_t;

#endif