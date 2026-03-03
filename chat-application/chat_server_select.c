#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "protocol.h"
#include "net_utils.h"
#include "monitor.h"

typedef struct {
    int  fd;
    char username[MAX_NAME];
    char status[16];
    int  active;
    int  authenticated;
} sel_slot_t;

sel_slot_t slots[MAX_USERS];

void broadcast_all(const char *sender, const char *text, int exclude_fd) {
    broadcast_payload_t pkt;
    strncpy(pkt.sender, sender, MAX_NAME);
    strncpy(pkt.text,   text,   BUFFER_SIZE);
    for (int i = 0; i < MAX_USERS; i++) {
        if (slots[i].active && slots[i].authenticated && slots[i].fd != exclude_fd)
            send_msg(slots[i].fd, MSG_BROADCAST, &pkt, sizeof(pkt));
    }
}

int main() {
    start_monitor("metrics_select.log");
    signal(SIGPIPE, SIG_IGN);

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(CHAT_PORT);

    bind(srv, (struct sockaddr *)&addr, sizeof(addr));
    listen(srv, MAX_USERS);
    printf("[Select Server] Listening on port %d\n", CHAT_PORT);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(srv, &readfds);
        int max_fd = srv;

        for (int i = 0; i < MAX_USERS; i++) {
            if (slots[i].active) {
                FD_SET(slots[i].fd, &readfds);
                if (slots[i].fd > max_fd) max_fd = slots[i].fd;
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(srv, &readfds)) {
            struct sockaddr_in caddr;
            socklen_t clen = sizeof(caddr);
            int cfd = accept(srv, (struct sockaddr *)&caddr, &clen);
            int added = 0;
            for (int i = 0; i < MAX_USERS; i++) {
                if (!slots[i].active) {
                    slots[i].fd            = cfd;
                    slots[i].active        = 1;
                    slots[i].authenticated = 0;
                    strcpy(slots[i].status, "available");
                    added = 1;
                    break;
                }
            }
            if (!added) { send_msg(cfd, MSG_ERROR, "full", 4); close(cfd); }
        }

        for (int i = 0; i < MAX_USERS; i++) {
            if (!slots[i].active) continue;
            if (!FD_ISSET(slots[i].fd, &readfds)) continue;

            uint8_t type;
            char buf[BUFFER_SIZE * 2];
            int r = recv_msg(slots[i].fd, &type, buf, sizeof(buf));

            if (r <= 0) {
                char leave[64];
                snprintf(leave, sizeof(leave), "%s left", slots[i].username);
                broadcast_all("server", leave, slots[i].fd);
                close(slots[i].fd);
                slots[i].active = 0;
                continue;
            }

            if (!slots[i].authenticated) {
                if (type == MSG_LOGIN) {
                    login_payload_t *lp = (login_payload_t *)buf;
                    strncpy(slots[i].username, lp->username, MAX_NAME);
                    slots[i].authenticated = 1;
                    send_msg(slots[i].fd, MSG_AUTH_OK, "welcome", 7);
                    printf("[Select] %s authenticated\n", lp->username);
                    char join[64];
                    snprintf(join, sizeof(join), "%s joined", lp->username);
                    broadcast_all("server", join, slots[i].fd);
                }
                continue;
            }

            if (type == MSG_BROADCAST) {
                broadcast_payload_t *bp = (broadcast_payload_t *)buf;
                broadcast_all(bp->sender, bp->text, slots[i].fd);
            }
            else if (type == MSG_PRIVATE) {
                private_payload_t *pp = (private_payload_t *)buf;
                for (int j = 0; j < MAX_USERS; j++) {
                    if (slots[j].active && strcmp(slots[j].username, pp->recipient) == 0) {
                        send_msg(slots[j].fd, MSG_PRIVATE, pp, sizeof(*pp));
                        break;
                    }
                }
            }
            else if (type == MSG_LIST_USERS) {
                char list[BUFFER_SIZE * 2] = {0};
                for (int j = 0; j < MAX_USERS; j++) {
                    if (slots[j].active && slots[j].authenticated) {
                        strncat(list, slots[j].username, sizeof(list) - strlen(list) - 1);
                        strncat(list, " [", 2);
                        strncat(list, slots[j].status, sizeof(list) - strlen(list) - 1);
                        strncat(list, "]\n", 2);
                    }
                }
                send_msg(slots[i].fd, MSG_USER_LIST, list, strlen(list));
            }
            else if (type == MSG_STATUS) {
                status_payload_t *sp = (status_payload_t *)buf;
                strncpy(slots[i].status, sp->status, 16);
            }
            else if (type == MSG_DISCONNECT) {
                char leave[64];
                snprintf(leave, sizeof(leave), "%s left", slots[i].username);
                broadcast_all("server", leave, slots[i].fd);
                close(slots[i].fd);
                slots[i].active = 0;
            }
        }
    }
}