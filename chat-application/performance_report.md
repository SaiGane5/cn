# Performance Analysis Report
## CS 3205 Assignment 2 — Multi-Client Chat System

---

## 1. Methodology

### Test Environment
- Platform: macOS Darwin 25.3.0 (loopback interface, 127.0.0.1)
- All clients and server run on the same host (eliminates network jitter)
- Three server implementations tested: **fork-based**, **thread-based**, **select-based**
- Each server listens on TCP port 8080

### Monitoring Module
A dedicated background thread (`monitor.c`) reads `/proc`-equivalent metrics every 5 seconds and appends CSV rows to `metrics_{type}.log`:

```
timestamp_ms, vmrss_kb, pss_kb, cpu_pct
```

`VmRSS` = Resident Set Size (physical RAM held by the process).
`PSS` = Proportional Set Size (shared pages counted proportionally).

### Load Test Design
- **Script**: `test_load.sh [fork|thread|select]`
- 5 **receiver** clients connect and wait for messages
- 5 **sender** clients each send 50 broadcast messages with an embedded millisecond timestamp
- Receivers parse the timestamp on arrival and compute one-way latency: `recv_time_ms − send_time_ms`
- 250 latency samples total per server type
- Measures: server processing + socket I/O overhead over loopback

### Stress Test Design
- **Script**: `test_stress.sh [fork|thread|select]`
- Concurrent clients ramped: 1 → 2 → 4 → 6 → 8 → 10
- Each client holds an open TCP connection and broadcasts a message every 0.5 s
- 8-second sampling window at each level for the monitor to capture stable metrics
- Captures: VmRSS (kB), PSS (kB), CPU (%)

---

## 2. Test Results

### 2.1 Message Delivery Latency (Load Test)

| Server | Samples | Min (ms) | Median (ms) | Mean (ms) | Max (ms) | p95 (ms) |
|--------|---------|----------|-------------|-----------|----------|----------|
| Fork   | 247 | 0 | 1 | 0.62 | 2 | 1 |
| Thread | 250 | 0 | 0 | 0.50 | 1 | 1 |
| Select | 250 | 0 | 0 | 0.39 | 2 | 1 |

**Visualization**: `latency_distribution.png`

All three stay in the sub-millisecond to 1 ms range on loopback. Select-based leads marginally due to zero per-client process/thread allocation overhead. Fork has the highest mean (0.62 ms) caused by occasional Linux scheduler delays when waking a child process to forward the broadcast.

### 2.2 CPU Usage vs Concurrent Clients (Stress Test)

| Clients | Fork CPU (%) | Thread CPU (%) | Select CPU (%) |
|---------|-------------|----------------|----------------|
| 1  | 0.02 | 0.01 | 0.00 |
| 2  | 0.02 | 0.02 | 0.02 |
| 4  | 0.05 | 0.08 | 0.06 |
| 6  | 0.06 | 0.12 | 0.11 |
| 8  | 0.09 | 0.20 | 0.10 |
| 10 | 0.15 | 0.28 | 0.20 |

### 2.3 Memory Usage (VmRSS) vs Concurrent Clients (Stress Test)

| Clients | Fork RSS (kB) | Thread RSS (kB) | Select RSS (kB) |
|---------|--------------|-----------------|-----------------|
| 1  | 1056 | 960  | 1248 |
| 2  | 1056 | 1008 | 864  |
| 4  | 1056 | 1168 | 864  |
| 6  | 1056 | 1344 | 864  |
| 8  | 1056 | 1552 | 864  |
| 10 | 1056 | 1792 | 864  |

**Visualization**: `cpu_memory_comparison.png`

---

## 3. Analysis and Observations

### Latency
- All three servers deliver messages in ≤ 2 ms on loopback, confirming correctness of the protocol framing and broadcast logic.
- **Select** achieves the lowest mean latency (0.39 ms): the single process never context-switches between OS-scheduled entities; the `select()` loop dispatches all I/O in one tight polling pass.
- **Thread** (0.50 ms mean) performs well because threads share the same address space, avoiding the IPC overhead inherent in forking.
- **Fork** (0.62 ms mean) incurs the highest latency due to broadcast messages needing to be written from the child's context through shared memory or re-sent per child, compounded by OS scheduler wake-up latency for child processes.

### Memory
- **Fork** memory stays flat (1056 kB) regardless of client count because each client is a separate process — the *server* process itself does not grow; the children are not reflected in the parent's VmRSS as measured here.
- **Thread** memory grows linearly (~160–180 kB per additional pair of clients) because each pthread requires its own 8 MB default stack allocated in the *same* process address space.
- **Select** is the most memory-efficient single-process model (864 kB flat) since no per-client stack or address space is allocated; all state lives in heap arrays.

### CPU
- At 10 clients the thread server uses the most CPU (0.28%) because of mutex contention (`slots_lock`) serialising every broadcast across 10 threads.
- Select CPU scales slowly (0.20% at 10 clients) since there is no mutex contention; all operations are single-threaded.
- Fork CPU is lowest (0.15%) at 10 clients partly because broadcast from child processes does not contend with each other — each child writes independently.

---

## 4. Bottlenecks Identified

| Bottleneck | Server | Description |
|------------|--------|-------------|
| Mutex contention | Thread | `slots_lock` is held during entire broadcast loop; all threads block when any one broadcasts. |
| Per-thread stack | Thread | Default 8 MB stack per thread means memory grows ~160 kB/client measured; at scale this becomes the limiting factor. |
| `select()` fd limit | Select | `FD_SETSIZE` is 1024 on most systems; the select-based server cannot serve more than ~1020 simultaneous clients without switching to `poll()`/`epoll`. |
| Child-process broadcast | Fork | Each child must iterate over shared memory to notify peer children; relies on shared `slots[]` array which can cause race conditions if not carefully locked. |
| In-memory history | Thread | History buffer is capped at 4096 entries with no persistence; server restart loses all history. |

---

## 5. Potential Optimizations

1. **Thread server — fine-grained locking**: Replace the single `slots_lock` with per-slot locks or a reader-writer lock (`pthread_rwlock_t`) to allow concurrent reads of the slot table during broadcast.
2. **Thread server — thread pool**: Pre-spawn a fixed pool of worker threads instead of creating one per connection to eliminate thread creation overhead and cap memory usage.
3. **Select server → epoll**: Replace `select()` with Linux `epoll` to remove the 1024 fd limit and reduce O(n) scan cost on each iteration.
4. **Fork server — pipe-based broadcast**: Use a shared pipe or UNIX socket pair between child processes for broadcast instead of shared memory, eliminating the need for cross-process locking.
5. **Persistent chat history**: Write history entries to a file (JSON or binary) so they survive server restarts. Use an append-only log for minimal write overhead.
6. **Message queuing**: Add a per-client send queue (linked list + condition variable) in the thread server so a slow client cannot stall the `slots_lock` during broadcast.
