#!/bin/bash
# Load test: measures actual one-way message delivery latency.
# Usage: ./test_load.sh [fork|thread|select]
#
# Strategy: sender clients embed the send-timestamp in each broadcast
# message's text field.  Receiver clients parse that timestamp on
# receipt and compute one-way delivery latency.  Both sides share the
# same system clock (loopback), so no clock-sync issue.
#
# Latency = time message exits the kernel send buffer on the sender
#           to the time it is read out of the kernel recv buffer on
#           the receiver — i.e., server processing + socket overhead.

SERVER_TYPE=${1:-fork}
PORT=8080
LOGDIR="latency_logs_${SERVER_TYPE}"
mkdir -p "$LOGDIR"

echo "Starting load test for '$SERVER_TYPE' server (port $PORT)..."
echo "Logs will be written to $LOGDIR/"

# ------------------------------------------------------------------ #
# Step 1: launch RECEIVER clients (recv1..recv5)
#         They listen for broadcast messages from their paired sender,
#         extract the embedded timestamp, and log delivery latency.
# ------------------------------------------------------------------ #
for i in $(seq 1 5); do
    (
        python3 - <<PYEOF
import socket, struct, time, os, sys

def send_msg(s, mtype, payload=b""):
    if isinstance(payload, str): payload = payload.encode()
    s.sendall(struct.pack("!BL", mtype, len(payload)) + payload)

def recv_msg(s):
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

s = socket.socket()
s.settimeout(30)
s.connect(("127.0.0.1", $PORT))

uname  = "recv$i".encode().ljust(32, b'\x00')
passwd = b"pass".ljust(32, b'\x00')
send_msg(s, 2, uname + passwd)   # MSG_LOGIN
recv_msg(s)                       # AUTH_OK

latencies = []
s.settimeout(90)
try:
    while len(latencies) < 50:
        mtype, data = recv_msg(s)
        if mtype is None:
            break
        if mtype == 3 and data and len(data) >= 32 + 32:
            # broadcast_payload_t: sender[32] + text[1024]
            sender_field = data[:32].rstrip(b'\x00').decode(errors='replace')
            text_field   = data[32:32+1024].rstrip(b'\x00').decode(errors='replace')
            # Only count messages from our paired sender
            if sender_field == "send$i" and text_field.startswith("ts:"):
                recv_time_ms = int(time.time() * 1000)
                try:
                    parts = text_field.split(":")
                    sent_time_ms = int(parts[1])
                    latency = recv_time_ms - sent_time_ms
                    if 0 <= latency < 10000:   # sanity: ignore bogus values
                        latencies.append(latency)
                except (IndexError, ValueError):
                    pass
except Exception as e:
    sys.stderr.write(f"[recv$i] exception: {e}\n")

logfile = "$LOGDIR/client_$i.csv"
with open(logfile, "w") as f:
    f.write("latency_ms\n")
    for l in latencies:
        f.write(f"{l}\n")

valid = [l for l in latencies if l >= 0]
med   = sorted(valid)[len(valid)//2] if valid else -1
print(f"[recv$i] logged {len(latencies)} latency samples, median={med} ms")
send_msg(s, 6, b"")
s.close()
PYEOF
    ) &
done

sleep 2   # give receivers time to authenticate before senders start

# ------------------------------------------------------------------ #
# Step 2: launch SENDER clients (send1..send5)
#         Each sends 50 broadcast messages with an embedded timestamp.
# ------------------------------------------------------------------ #
for i in $(seq 1 5); do
    (
        python3 - <<PYEOF
import socket, struct, time

def send_msg(s, mtype, payload=b""):
    if isinstance(payload, str): payload = payload.encode()
    s.sendall(struct.pack("!BL", mtype, len(payload)) + payload)

def recv_msg(s):
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

s = socket.socket()
s.settimeout(15)
s.connect(("127.0.0.1", $PORT))

uname  = "send$i".encode().ljust(32, b'\x00')
passwd = b"pass".ljust(32, b'\x00')
send_msg(s, 2, uname + passwd)   # MSG_LOGIN
recv_msg(s)                       # AUTH_OK

for j in range(50):
    ts_ms   = int(time.time() * 1000)
    # broadcast_payload_t: sender[32] + text[1024]
    # Embed send timestamp in text as "ts:{ms}:msg{j}"
    sender  = "send$i".encode().ljust(32, b'\x00')
    text    = f"ts:{ts_ms}:msg{j}".encode().ljust(1024, b'\x00')
    send_msg(s, 3, sender + text)   # MSG_BROADCAST
    # Small gap so receiver isn't overwhelmed; still fast enough to see
    # queuing differences across server implementations
    time.sleep(0.02)

print(f"[send$i] sent 50 messages")
send_msg(s, 6, b"")
s.close()
PYEOF
    ) &
done

wait
echo "Load test complete. Latency logs written to $LOGDIR/"
