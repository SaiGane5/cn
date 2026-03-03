#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include "protocol.h"
#include "net_utils.h"
#include "monitor.h"

typedef struct {
    int  fd;
    char username[MAX_NAME];
    char status[16];
    int  active;
} client_slot_t;

client_slot_t slots[MAX_USERS];
pthread_mutex_t slots_lock = PTHREAD_MUTEX_INITIALIZER;

/* chat history - bonus */
typedef struct {
    long long  ts;
    char       sender[MAX_NAME];
    char       recipient[MAX_NAME]; /* empty = broadcast */
    char       text[BUFFER_SIZE];
} history_entry_t;

history_entry_t history[4096];
int history_count = 0;
pthread_mutex_t hist_lock = PTHREAD_MUTEX_INITIALIZER;

void store_history(const char *sender, const char *recipient, const char *text) {
    pthread_mutex_lock(&hist_lock);
    if (history_count < 4096) {
        history_entry_t *e = &history[history_count++];
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        e->ts = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        strncpy(e->sender,    sender,    MAX_NAME);
        strncpy(e->recipient, recipient ? recipient : "", MAX_NAME);
        strncpy(e->text,      text,      BUFFER_SIZE);
    }
    pthread_mutex_unlock(&hist_lock);
}

void broadcast_msg(const char *sender, const char *text, int exclude_fd) {
    broadcast_payload_t pkt;
    strncpy(pkt.sender, sender, MAX_NAME);
    strncpy(pkt.text,   text,   BUFFER_SIZE);

    pthread_mutex_lock(&slots_lock);
    for (int i = 0; i < MAX_USERS; i++) {
        if (slots[i].active && slots[i].fd != exclude_fd) {
            send_msg(slots[i].fd, MSG_BROADCAST, &pkt, sizeof(pkt));
        }
    }
    pthread_mutex_unlock(&slots_lock);
    store_history(sender, NULL, text);
}

void private_msg(const char *sender, const char *recipient, const char *text) {
    private_payload_t pkt;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    pkt.timestamp = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    strncpy(pkt.sender,    sender,    MAX_NAME);
    strncpy(pkt.recipient, recipient, MAX_NAME);
    strncpy(pkt.text,      text,      BUFFER_SIZE);

    int found = 0;
    pthread_mutex_lock(&slots_lock);
    for (int i = 0; i < MAX_USERS; i++) {
        if (slots[i].active && strcmp(slots[i].username, recipient) == 0) {
            send_msg(slots[i].fd, MSG_PRIVATE, &pkt, sizeof(pkt));
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&slots_lock);

    if (!found) {
        char err[64];
        snprintf(err, sizeof(err), "User %s not online", recipient);
        /* find sender fd and notify */
        pthread_mutex_lock(&slots_lock);
        for (int i = 0; i < MAX_USERS; i++) {
            if (slots[i].active && strcmp(slots[i].username, sender) == 0) {
                send_msg(slots[i].fd, MSG_ERROR, err, strlen(err));
                break;
            }
        }
        pthread_mutex_unlock(&slots_lock);
    }
    store_history(sender, recipient, text);
}

typedef struct { int fd; struct sockaddr_in addr; } thread_arg_t;

void *handle_client(void *arg) {
    thread_arg_t *ta = (thread_arg_t *)arg;
    int fd = ta->fd;
    free(ta);

    /* authenticate */
    uint8_t type;
    char buf[BUFFER_SIZE * 2];
    if (recv_msg(fd, &type, buf, sizeof(buf)) <= 0 || type != MSG_LOGIN) {
        close(fd); return NULL;
    }
    login_payload_t *lp = (login_payload_t *)buf;

    /* assign slot */
    int slot = -1;
    pthread_mutex_lock(&slots_lock);
    for (int i = 0; i < MAX_USERS; i++) {
        if (!slots[i].active) { slot = i; break; }
    }
    if (slot != -1) {
        slots[slot].fd     = fd;
        slots[slot].active = 1;
        strncpy(slots[slot].username, lp->username, MAX_NAME);
        strcpy(slots[slot].status, "available");
    }
    pthread_mutex_unlock(&slots_lock);

    if (slot == -1) {
        send_msg(fd, MSG_ERROR, "server full", 11);
        close(fd); return NULL;
    }

    send_msg(fd, MSG_AUTH_OK, "welcome", 7);
    printf("[Thread] %s connected\n", lp->username);

    char join_note[64];
    snprintf(join_note, sizeof(join_note), "%s joined the chat", lp->username);
    broadcast_msg("server", join_note, fd);

    /* main recv loop */
    while (recv_msg(fd, &type, buf, sizeof(buf)) > 0) {
        if (type == MSG_BROADCAST) {
            broadcast_payload_t *bp = (broadcast_payload_t *)buf;
            broadcast_msg(bp->sender, bp->text, fd);
        }
        else if (type == MSG_PRIVATE) {
            private_payload_t *pp = (private_payload_t *)buf;
            private_msg(pp->sender, pp->recipient, pp->text);
        }
        else if (type == MSG_LIST_USERS) {
            char list[BUFFER_SIZE * 2] = {0};
            pthread_mutex_lock(&slots_lock);
            for (int i = 0; i < MAX_USERS; i++) {
                if (slots[i].active) {
                    strncat(list, slots[i].username, sizeof(list) - strlen(list) - 1);
                    strncat(list, " [", 2);
                    strncat(list, slots[i].status, sizeof(list) - strlen(list) - 1);
                    strncat(list, "]\n", 2);
                }
            }
            pthread_mutex_unlock(&slots_lock);
            send_msg(fd, MSG_USER_LIST, list, strlen(list));
        }
        else if (type == MSG_STATUS) {
            status_payload_t *sp = (status_payload_t *)buf;
            pthread_mutex_lock(&slots_lock);
            strncpy(slots[slot].status, sp->status, 16);
            pthread_mutex_unlock(&slots_lock);
        }
        else if (type == MSG_HISTORY_REQ) {
            char hbuf[BUFFER_SIZE * 8] = {0};
            pthread_mutex_lock(&hist_lock);
            for (int i = 0; i < history_count; i++) {
                history_entry_t *e = &history[i];
                if (strcmp(e->sender, slots[slot].username) == 0 ||
                    strcmp(e->recipient, slots[slot].username) == 0) {
                    char line[BUFFER_SIZE + 128];
                    snprintf(line, sizeof(line), "[%lld] %s -> %s: %s\n",
                             e->ts, e->sender,
                             e->recipient[0] ? e->recipient : "all",
                             e->text);
                    strncat(hbuf, line, sizeof(hbuf) - strlen(hbuf) - 1);
                }
            }
            pthread_mutex_unlock(&hist_lock);
            send_msg(fd, MSG_HISTORY_RES, hbuf, strlen(hbuf));
        }
        else if (type == MSG_DISCONNECT) {
            break;
        }
    }

    /* cleanup */
    printf("[Thread] %s disconnected\n", slots[slot].username);
    char leave[64];
    snprintf(leave, sizeof(leave), "%s left the chat", slots[slot].username);

    pthread_mutex_lock(&slots_lock);
    slots[slot].active = 0;
    pthread_mutex_unlock(&slots_lock);

    broadcast_msg("server", leave, fd);
    close(fd);
    return NULL;
}

int main() {
    start_monitor("metrics_thread.log");

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(CHAT_PORT);

    bind(srv,  (struct sockaddr *)&addr, sizeof(addr));
    listen(srv, MAX_USERS);
    printf("[Thread Server] Listening on port %d\n", CHAT_PORT);

    while (1) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int cfd = accept(srv, (struct sockaddr *)&caddr, &clen);
        if (cfd < 0) continue;

        thread_arg_t *ta = malloc(sizeof(thread_arg_t));
        ta->fd   = cfd;
        ta->addr = caddr;

        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&tid, &attr, handle_client, ta);
        pthread_attr_destroy(&attr);
    }
}