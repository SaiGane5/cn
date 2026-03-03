#!/bin/bash
# Stress test: gradually increase concurrent clients (1→10) and capture
# CPU/memory metrics at each load level.
# Usage: ./test_stress.sh [fork|thread|select]
#
# Design: at each level, clients are kept CONNECTED and continuously
# sending messages during the sampling window so the metrics reflect
# real concurrent load.  After the window, all clients disconnect and
# the next level begins.
#
# Output files:
#   metrics_{type}.log           – raw per-sample metrics (written by server)
#   stress_summary_{type}.log    – (num_clients, vmrss_kb, pss_kb, cpu_pct)
#                                  one row per load level; used by plot_results.py

SERVER_TYPE=${1:-fork}
PORT=8080
METRICS_LOG="metrics_${SERVER_TYPE}.log"
SUMMARY_LOG="stress_summary_${SERVER_TYPE}.log"
SAMPLE_WINDOW=8   # seconds to hold load while metrics are sampled (monitor writes every 5s)

echo "num_clients,vmrss_kb,pss_kb,cpu_pct" > "$SUMMARY_LOG"

echo "Starting stress test for '$SERVER_TYPE' server..."

for NUM_CLIENTS in 1 2 4 6 8 10; do
    echo "=== $NUM_CLIENTS concurrent clients ==="

    # Start NUM_CLIENTS clients; each keeps the connection alive,
    # sends a broadcast every 0.5 s for SAMPLE_WINDOW seconds, then exits.
    for i in $(seq 1 $NUM_CLIENTS); do
        (
            python3 - <<PYEOF
import socket, struct, time

def send_msg(s, mtype, payload=b""):
    if isinstance(payload, str): payload = payload.encode()
    s.sendall(struct.pack("!BL", mtype, len(payload)) + payload)

def recv_msg_nonblock(s):
    try:
        raw = b""
        while len(raw) < 5:
            chunk = s.recv(5 - len(raw))
            if not chunk: return None, None
            raw += chunk
        mtype, length = struct.unpack("!BL", raw)
        data = b""
        while len(data) < length:
            chunk = s.recv(length - len(data))
            if not chunk: break
            data += chunk
        return mtype, data
    except:
        return None, None

s = socket.socket()
s.connect(("127.0.0.1", $PORT))
uname  = f"stress$i".encode().ljust(32, b'\x00')
passwd = b"pass".ljust(32, b'\x00')
send_msg(s, 2, uname + passwd)
s.settimeout(1)
try: recv_msg_nonblock(s)
except: pass

deadline = time.time() + $SAMPLE_WINDOW
msg_num  = 0
s.settimeout(0.5)
while time.time() < deadline:
    sender = f"stress$i".encode().ljust(32, b'\x00')
    text   = f"load msg {msg_num}".encode().ljust(1024, b'\x00')
    try:
        send_msg(s, 3, sender + text)
    except:
        break
    msg_num += 1
    try: recv_msg_nonblock(s)
    except: pass
    time.sleep(0.5)

send_msg(s, 6, b"")
s.close()
PYEOF
        ) &
    done

    sleep $SAMPLE_WINDOW   # wait for load to build and metrics to be captured

    # Grab the most recent metrics sample from the server's log
    if [ -f "$METRICS_LOG" ]; then
        LAST=$(tail -1 "$METRICS_LOG")
        # Format: timestamp_ms,vmrss_kb,pss_kb,cpu_pct
        VMRSS=$(echo "$LAST" | cut -d',' -f2)
        PSS=$(echo   "$LAST" | cut -d',' -f3)
        CPU=$(echo   "$LAST" | cut -d',' -f4)
        echo "$NUM_CLIENTS,$VMRSS,$PSS,$CPU" >> "$SUMMARY_LOG"
        echo "  Captured: clients=$NUM_CLIENTS vmrss=${VMRSS}kB pss=${PSS}kB cpu=${CPU}%"
    else
        echo "  Warning: $METRICS_LOG not found; is the server running?"
        echo "$NUM_CLIENTS,0,0,0.00" >> "$SUMMARY_LOG"
    fi

    wait   # let all clients finish cleanly
    sleep 2
done

echo "Stress test done."
echo "Summary written to $SUMMARY_LOG"
