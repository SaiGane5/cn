#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "protocol.h"
#include "net_utils.h"

typedef struct {
    char     username[MAX_NAME];
    char     password[MAX_PASS];
    char     ip[INET_ADDRSTRLEN];
    uint16_t port;
    int      active;
} user_record_t;

user_record_t users[MAX_USERS];
int user_count = 0;
pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

int find_user(const char *uname) {
    for (int i = 0; i < user_count; i++)
        if (strcmp(users[i].username, uname) == 0) return i;
    return -1;
}

void *handle_client(void *arg) {
    int fd = *(int *)arg;
    free(arg);

    uint8_t type;
    char buf[sizeof(reg_payload_t) + 64];

    while (recv_msg(fd, &type, buf, sizeof(buf)) > 0) {
        if (type == MSG_REGISTER) {
            reg_payload_t *r = (reg_payload_t *)buf;
            pthread_mutex_lock(&db_lock);
            int idx = find_user(r->username);
            if (idx == -1) {
                idx = user_count++;
                strncpy(users[idx].username, r->username, MAX_NAME);
                strncpy(users[idx].password, r->password, MAX_PASS);
                users[idx].port = r->port;
                users[idx].active = 1;
                printf("[Discovery] Registered: %s\n", r->username);
                send_msg(fd, MSG_AUTH_OK, "registered", 10);
            } else {
                /* already exists: update port if password matches */
                if (strcmp(users[idx].password, r->password) == 0) {
                    users[idx].port = r->port;
                    users[idx].active = 1;
                    send_msg(fd, MSG_AUTH_OK, "updated", 7);
                } else {
                    send_msg(fd, MSG_AUTH_FAIL, "bad pass", 8);
                }
            }
            pthread_mutex_unlock(&db_lock);
        }
        else if (type == MSG_LIST_USERS) {
            /* build a newline-separated list of user:ip:port */
            char list[BUFFER_SIZE * 4] = {0};
            pthread_mutex_lock(&db_lock);
            for (int i = 0; i < user_count; i++) {
                if (users[i].active) {
                    char entry[128];
                    snprintf(entry, sizeof(entry), "%s:%s:%d\n",
                             users[i].username, users[i].ip, users[i].port);
                    strncat(list, entry, sizeof(list) - strlen(list) - 1);
                }
            }
            pthread_mutex_unlock(&db_lock);
            send_msg(fd, MSG_USER_LIST, list, strlen(list));
        }
    }

    close(fd);
    return NULL;
}

int main() {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(DISCOVERY_PORT);

    bind(srv, (struct sockaddr *)&addr, sizeof(addr));
    listen(srv, 10);
    printf("[Discovery] Listening on port %d\n", DISCOVERY_PORT);

    while (1) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int cfd = accept(srv, (struct sockaddr *)&caddr, &clen);
        if (cfd < 0) continue;

        int *pfd = malloc(sizeof(int));
        *pfd = cfd;
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, pfd);
        pthread_detach(tid);
    }
}