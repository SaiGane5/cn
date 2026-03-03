#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "protocol.h"
#include "net_utils.h"
#include "monitor.h"

/*
 * IPC Strategy: Unix domain socket pipe back to parent.
 * Each child sends routed messages to the parent's IPC socket.
 * Parent holds a table of (username -> client_fd) and forwards.
 * This is necessary because fork() gives each child a separate
 * address space — child A cannot directly access child B's fd.
 */

#define IPC_PATH "/tmp/chat_fork_ipc.sock"

typedef struct {
    char sender[MAX_NAME];
    char recipient[MAX_NAME]; /* empty = broadcast */
    char text[BUFFER_SIZE];
    uint8_t msg_type;
} ipc_msg_t;

typedef struct {
    char username[MAX_NAME];
    int  fd;
    int  active;
} parent_slot_t;

parent_slot_t pslots[MAX_USERS];
int ipc_srv_fd = -1;

void sigchld_handler(int s) {
    (void)s;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void child_handle_client(int cfd, int ipc_fd, const char *username) {
    uint8_t type;
    char buf[BUFFER_SIZE * 2];

    while (recv_msg(cfd, &type, buf, sizeof(buf)) > 0) {
        ipc_msg_t imsg = {0};
        imsg.msg_type = type;

        if (type == MSG_BROADCAST) {
            broadcast_payload_t *bp = (broadcast_payload_t *)buf;
            strncpy(imsg.sender, bp->sender, MAX_NAME);
            strncpy(imsg.text,   bp->text,   BUFFER_SIZE);
        }
        else if (type == MSG_PRIVATE) {
            private_payload_t *pp = (private_payload_t *)buf;
            strncpy(imsg.sender,    pp->sender,    MAX_NAME);
            strncpy(imsg.recipient, pp->recipient, MAX_NAME);
            strncpy(imsg.text,      pp->text,      BUFFER_SIZE);
        }
        else if (type == MSG_LIST_USERS || type == MSG_DISCONNECT) {
            strncpy(imsg.sender, username, MAX_NAME);
        }

        write(ipc_fd, &imsg, sizeof(imsg));

        if (type == MSG_DISCONNECT) break;
    }

    /* notify parent of disconnect */
    ipc_msg_t imsg = {0};
    imsg.msg_type = MSG_DISCONNECT;
    strncpy(imsg.sender, username, MAX_NAME);
    write(ipc_fd, &imsg, sizeof(imsg));
    close(ipc_fd);
    close(cfd);
}

int find_slot(const char *uname) {
    for (int i = 0; i < MAX_USERS; i++)
        if (pslots[i].active && strcmp(pslots[i].username, uname) == 0)
            return i;
    return -1;
}

void parent_broadcast(const char *sender, const char *text, const char *exclude) {
    broadcast_payload_t pkt;
    strncpy(pkt.sender, sender, MAX_NAME);
    strncpy(pkt.text,   text,   BUFFER_SIZE);
    for (int i = 0; i < MAX_USERS; i++) {
        if (pslots[i].active && strcmp(pslots[i].username, exclude) != 0)
            send_msg(pslots[i].fd, MSG_BROADCAST, &pkt, sizeof(pkt));
    }
}

int main() {
    start_monitor("metrics_fork.log");

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    /* IPC listener (Unix socket) */
    unlink(IPC_PATH);
    ipc_srv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un ipc_addr = {0};
    ipc_addr.sun_family = AF_UNIX;
    strncpy(ipc_addr.sun_path, IPC_PATH, sizeof(ipc_addr.sun_path) - 1);
    bind(ipc_srv_fd, (struct sockaddr *)&ipc_addr, sizeof(ipc_addr));
    listen(ipc_srv_fd, MAX_USERS);

    /* TCP listener */
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(CHAT_PORT);
    bind(srv, (struct sockaddr *)&addr, sizeof(addr));
    listen(srv, MAX_USERS);
    printf("[Fork Server] Listening on port %d\n", CHAT_PORT);

    fd_set readfds;
    int max_fd;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(srv, &readfds);
        FD_SET(ipc_srv_fd, &readfds);
        max_fd = srv > ipc_srv_fd ? srv : ipc_srv_fd;

        /* also watch active IPC client fds (children talking back) */
        for (int i = 0; i < MAX_USERS; i++) {
            if (pslots[i].active) {
                FD_SET(pslots[i].fd, &readfds);
                if (pslots[i].fd > max_fd) max_fd = pslots[i].fd;
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        /* new TCP client */
        if (FD_ISSET(srv, &readfds)) {
            struct sockaddr_in caddr;
            socklen_t clen = sizeof(caddr);
            int cfd = accept(srv, (struct sockaddr *)&caddr, &clen);

            /* authenticate before forking */
            uint8_t type;
            char buf[sizeof(login_payload_t)];
            recv_msg(cfd, &type, buf, sizeof(buf));
            login_payload_t *lp = (login_payload_t *)buf;
            send_msg(cfd, MSG_AUTH_OK, "welcome", 7);

            int slot = -1;
            for (int i = 0; i < MAX_USERS; i++) {
                if (!pslots[i].active) { slot = i; break; }
            }

            if (slot == -1) {
                send_msg(cfd, MSG_ERROR, "full", 4);
                close(cfd); continue;
            }

            /* IPC: child will connect back to parent's Unix socket */
            pid_t pid = fork();
            if (pid == 0) {
                close(srv);
                close(ipc_srv_fd);
                int ipc_fd = socket(AF_UNIX, SOCK_STREAM, 0);
                connect(ipc_fd, (struct sockaddr *)&ipc_addr, sizeof(ipc_addr));

                /* send username over IPC so parent knows who connected */
                write(ipc_fd, lp->username, MAX_NAME);

                child_handle_client(cfd, ipc_fd, lp->username);
                exit(0);
            } else {
                close(cfd);
                printf("[Fork] Forked PID %d for %s\n", pid, lp->username);
            }
        }

        /* new IPC connection from a child */
        if (FD_ISSET(ipc_srv_fd, &readfds)) {
            int ipc_cfd = accept(ipc_srv_fd, NULL, NULL);
            char uname[MAX_NAME];
            read(ipc_cfd, uname, MAX_NAME);
            for (int i = 0; i < MAX_USERS; i++) {
                if (!pslots[i].active) {
                    pslots[i].active = 1;
                    pslots[i].fd     = ipc_cfd;
                    strncpy(pslots[i].username, uname, MAX_NAME);
                    break;
                }
            }
        }

        /* messages from children over IPC */
        for (int i = 0; i < MAX_USERS; i++) {
            if (!pslots[i].active) continue;
            if (!FD_ISSET(pslots[i].fd, &readfds)) continue;

            ipc_msg_t imsg;
            int r = read(pslots[i].fd, &imsg, sizeof(imsg));
            if (r <= 0 || imsg.msg_type == MSG_DISCONNECT) {
                char leave[64];
                snprintf(leave, sizeof(leave), "%s left", pslots[i].username);
                parent_broadcast("server", leave, pslots[i].username);
                close(pslots[i].fd);
                pslots[i].active = 0;
                continue;
            }

            if (imsg.msg_type == MSG_BROADCAST) {
                parent_broadcast(imsg.sender, imsg.text, imsg.sender);
            }
            else if (imsg.msg_type == MSG_PRIVATE) {
                int idx = find_slot(imsg.recipient);
                if (idx != -1) {
                    private_payload_t pkt;
                    strncpy(pkt.sender,    imsg.sender,    MAX_NAME);
                    strncpy(pkt.recipient, imsg.recipient, MAX_NAME);
                    strncpy(pkt.text,      imsg.text,      BUFFER_SIZE);
                    /* parent routes to recipient's IPC fd */
                    ipc_msg_t fwd = imsg;
                    write(pslots[idx].fd, &fwd, sizeof(fwd));
                }
            }
            else if (imsg.msg_type == MSG_LIST_USERS) {
                char list[BUFFER_SIZE] = {0};
                for (int j = 0; j < MAX_USERS; j++) {
                    if (pslots[j].active) {
                        strncat(list, pslots[j].username, sizeof(list) - strlen(list) - 1);
                        strncat(list, "\n", 1);
                    }
                }
                /* write list back to requesting child's IPC fd */
                write(pslots[i].fd, list, strlen(list) + 1);
            }
        }
    }
}