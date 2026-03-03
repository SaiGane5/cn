CC      = gcc
CFLAGS  = -Wall -g
LDFLAGS = -lpthread

all: discovery_server chat_server_thread chat_server_fork chat_server_select chat_client

discovery_server: discovery_server.c net_utils.c monitor.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

chat_server_thread: chat_server_thread.c net_utils.c monitor.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

chat_server_fork: chat_server_fork.c net_utils.c monitor.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

chat_server_select: chat_server_select.c net_utils.c monitor.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

chat_client: chat_client.c net_utils.c monitor.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f discovery_server chat_server_thread chat_server_fork chat_server_select chat_client *.log