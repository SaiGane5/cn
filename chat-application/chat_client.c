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

int srv_fd;
char my_username[MAX_NAME];

/* background thread: listen for incoming messages */
void *recv_loop(void *arg) {
    (void)arg;
    uint8_t type;
    char buf[BUFFER_SIZE * 2];

    while (recv_msg(srv_fd, &type, buf, sizeof(buf)) > 0) {
        if (type == MSG_BROADCAST) {
            broadcast_payload_t *bp = (broadcast_payload_t *)buf;
            printf("\n[BROADCAST] %s: %s", bp->sender, bp->text);
        }
        else if (type == MSG_PRIVATE) {
            private_payload_t *pp = (private_payload_t *)buf;
            long long now = get_time_ms();
            long long latency = now - pp->timestamp;
            printf("\n[PM from %s] %s  (latency: %lldms)\n",
                   pp->sender, pp->text, latency);
        }
        else if (type == MSG_USER_LIST) {
            printf("\n[Online users]\n%s\n", buf);
        }
        else if (type == MSG_NOTIFY) {
            printf("\n[Server] %s\n", buf);
        }
        else if (type == MSG_ERROR) {
            printf("\n[Error] %s\n", buf);
        }
        else if (type == MSG_AUTH_OK) {
            printf("[Auth] %s\n", buf);
        }
        else if (type == MSG_HISTORY_RES) {
            printf("\n[History]\n%s\n", buf);
        }
        fflush(stdout);
    }
    printf("\nDisconnected from server.\n");
    exit(0);
    return NULL;
}

int connect_to_server(const char *ip, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect"); return -1;
    }
    return fd;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <username> <password> <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    strncpy(my_username, argv[1], MAX_NAME);
    const char *password   = argv[2];
    const char *server_ip  = argv[3];
    int         server_port = atoi(argv[4]);

    /* Step 1: register with discovery server */
    int disc_fd = connect_to_server("127.0.0.1", DISCOVERY_PORT);
    if (disc_fd >= 0) {
        reg_payload_t rp = {0};
        strncpy(rp.username, my_username, MAX_NAME);
        strncpy(rp.password, password, MAX_PASS);
        rp.port = server_port;
        send_msg(disc_fd, MSG_REGISTER, &rp, sizeof(rp));
        uint8_t type; char buf[64];
        recv_msg(disc_fd, &type, buf, sizeof(buf));
        close(disc_fd);
        printf("[Discovery] Registration: %s\n", buf);
    }

    /* Step 2: connect to chat server */
    srv_fd = connect_to_server(server_ip, server_port);
    if (srv_fd < 0) { fprintf(stderr, "Cannot connect to chat server\n"); return 1; }

    /* Step 3: login */
    login_payload_t lp = {0};
    strncpy(lp.username, my_username, MAX_NAME);
    strncpy(lp.password, password,    MAX_PASS);
    send_msg(srv_fd, MSG_LOGIN, &lp, sizeof(lp));

    /* start receiver thread */
    pthread_t rtid;
    pthread_create(&rtid, NULL, recv_loop, NULL);
    pthread_detach(rtid);

    /* CLI loop */
    char line[BUFFER_SIZE + 64];
    printf("Commands: /broadcast <msg>  /pm <user> <msg>  /list  /status <s>  /history  /quit\n");

    while (fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "/broadcast ", 11) == 0) {
            broadcast_payload_t pkt = {0};
            strncpy(pkt.sender, my_username, MAX_NAME);
            strncpy(pkt.text,   line + 11,   BUFFER_SIZE);
            send_msg(srv_fd, MSG_BROADCAST, &pkt, sizeof(pkt));
        }
        else if (strncmp(line, "/pm ", 4) == 0) {
            char recip[MAX_NAME]; char text[BUFFER_SIZE];
            sscanf(line + 4, "%31s %1023[^\n]", recip, text);
            private_payload_t pkt = {0};
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            pkt.timestamp = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
            strncpy(pkt.sender,    my_username, MAX_NAME);
            strncpy(pkt.recipient, recip,       MAX_NAME);
            strncpy(pkt.text,      text,        BUFFER_SIZE);
            send_msg(srv_fd, MSG_PRIVATE, &pkt, sizeof(pkt));
        }
        else if (strcmp(line, "/list") == 0) {
            send_msg(srv_fd, MSG_LIST_USERS, NULL, 0);
        }
        else if (strncmp(line, "/status ", 8) == 0) {
            status_payload_t pkt = {0};
            strncpy(pkt.username, my_username, MAX_NAME);
            strncpy(pkt.status,   line + 8,    16);
            send_msg(srv_fd, MSG_STATUS, &pkt, sizeof(pkt));
        }
        else if (strcmp(line, "/history") == 0) {
            send_msg(srv_fd, MSG_HISTORY_REQ, NULL, 0);
        }
        else if (strcmp(line, "/quit") == 0) {
            send_msg(srv_fd, MSG_DISCONNECT, NULL, 0);
            break;
        }
    }

    close(srv_fd);
    return 0;
}