# Multi-Client Chat System — CS 3205 Assignment 2

## System Architecture Overview

The system consists of four components:

```
┌──────────────────┐        ┌───────────────────────┐
│  Discovery Server│        │     Chat Server        │
│  Port: 9090      │        │  Port: 8080            │
│                  │        │  (fork / thread /      │
│  DNS-like service│        │   select variant)      │
│  username→IP:port│        │                        │
└──────┬───────────┘        └──────────┬────────────┘
       │ register/lookup               │ login / msgs
       ▼                               ▼
┌──────────────────────────────────────────────────┐
│                  Chat Client                     │
│  1. Registers with discovery server on startup   │
│  2. Connects to chat server with credentials     │
│  3. Interactive CLI: broadcast / private / list  │
└──────────────────────────────────────────────────┘
```

### Components

| File | Role |
|------|------|
| `discovery_server.c` | DNS-like registry: maps username → IP:port, handles registration |
| `chat_server_thread.c` | Chat server — one **pthread** per client |
| `chat_server_fork.c` | Chat server — one **forked process** per client |
| `chat_server_select.c` | Chat server — **non-blocking** with `select()` |
| `chat_client.c` | Interactive CLI client |
| `monitor.c / monitor.h` | Background metrics thread (CPU %, RSS, PSS) |
| `net_utils.c / net_utils.h` | Framed send/recv helpers |
| `protocol.h` | All message type constants and payload structs |

---

## Protocol Specification

Custom binary application-layer protocol over TCP.

### Message Frame

```
 0       1       2       3       4       5+
 +-------+-------+-------+-------+-------+--- ...
 | type  |    length (4 bytes, big-endian)|  payload
 +-------+-------+-------+-------+-------+--- ...
```

Every message starts with a 5-byte header (`msg_header_t`):

| Field | Size | Description |
|-------|------|-------------|
| `type` | 1 byte | Message type (see below) |
| `length` | 4 bytes (BE) | Payload byte count |

Followed by `length` bytes of payload.

### Message Types

| ID | Name | Direction | Payload |
|----|------|-----------|---------|
| 1  | `MSG_REGISTER` | client→discovery | `reg_payload_t` (username[32] + password[32] + port[2]) |
| 2  | `MSG_LOGIN` | client→chat | `login_payload_t` (username[32] + password[32]) |
| 3  | `MSG_BROADCAST` | client↔server | `broadcast_payload_t` (sender[32] + text[1024]) |
| 4  | `MSG_PRIVATE` | client↔server | `private_payload_t` (sender[32] + recipient[32] + text[1024] + timestamp[8]) |
| 5  | `MSG_LIST_USERS` | client→server | empty |
| 6  | `MSG_DISCONNECT` | client→server | empty |
| 7  | `MSG_AUTH_OK` | server→client | short string |
| 8  | `MSG_AUTH_FAIL` | server→client | short string |
| 9  | `MSG_USER_LIST` | server→client | newline-separated `user [status]` entries |
| 10 | `MSG_ERROR` | server→client | error string |
| 11 | `MSG_NOTIFY` | server→client | notification string |
| 12 | `MSG_STATUS` | client→server | `status_payload_t` (username[32] + status[16]) |
| 13 | `MSG_HISTORY_REQ` | client→server | empty |
| 14 | `MSG_HISTORY_RES` | server→client | newline-separated history lines |

---

## Compilation

Requires: `gcc`, `pthreads`, `make`

```bash
make all
```

Builds: `discovery_server`, `chat_server_thread`, `chat_server_fork`, `chat_server_select`, `chat_client`

To build a single binary:

```bash
make chat_server_thread
make chat_server_fork
make chat_server_select
```

To clean:

```bash
make clean
```

---

## Execution Instructions

### 1. Start Discovery Server

```bash
./discovery_server
# Listens on port 9090
```

### 2. Start a Chat Server (pick one variant)

```bash
./chat_server_thread   # pthreads, port 8080
./chat_server_fork     # fork,     port 8080
./chat_server_select   # select,   port 8080
```

### 3. Start Clients

```bash
./chat_client
```

The client prompts for username, password, and connects to both the discovery server and chat server.

**CLI commands inside the client:**

| Command | Action |
|---------|--------|
| `/broadcast <msg>` | Send message to all online users |
| `/msg <user> <msg>` | Send private message to `<user>` |
| `/list` | Show online users and their status |
| `/status <available\|busy\|away>` | Change your status |
| `/history` | View your chat history |
| `/quit` | Disconnect gracefully |

---

## Testing Guide

### Load Test (latency measurement)

Simulates 5 sender + 5 receiver clients, each pair exchanging 50 timestamped broadcast messages. Measures one-way delivery latency.

```bash
# Server must be running first
./chat_server_thread &
bash test_load.sh thread    # or fork / select
```

Latency CSVs saved to `latency_logs_thread/`.

### Stress Test (CPU/memory under increasing load)

Ramps concurrent clients from 1 → 2 → 4 → 6 → 8 → 10, sampling server metrics at each level.

```bash
./chat_server_thread &
bash test_stress.sh thread   # or fork / select
```

Summary saved to `stress_summary_thread.log`.

### Generate Plots

```bash
python3 plot_results.py
# Outputs: latency_distribution.png, cpu_memory_comparison.png
```
