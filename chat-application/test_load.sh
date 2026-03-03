#!/bin/bash
# Load test: 10 concurrent clients, 50 messages each
PORT=8080
LOGDIR="latency_logs"
mkdir -p $LOGDIR

echo "Starting load test..."

for i in $(seq 1 10); do
    (
        python3 - <<PYEOF
import socket, struct, time

def send_msg(s, mtype, payload=b""):
    if isinstance(payload, str):
        payload = payload.encode()
    s.sendall(struct.pack("!BL", mtype, len(payload)) + payload)

def recv_msg(s):
    raw = b""
    while len(raw) < 5:
        chunk = s.recv(5 - len(raw))
        if not chunk:
            return None, None
        raw += chunk
    mtype, length = struct.unpack("!BL", raw)
    data = b""
    while len(data) < length:
        chunk = s.recv(length - len(data))
        if not chunk:
            break
        data += chunk
    return mtype, data

idx = $i

try:
    s = socket.socket()
    s.settimeout(10)
    s.connect(("127.0.0.1", $PORT))

    # login_payload_t: username[32] + password[32]
    uname = f"user{idx}".encode().ljust(32, b'\x00')
    passwd = b"pass".ljust(32, b'\x00')
    send_msg(s, 2, uname + passwd)

    mtype, data = recv_msg(s)  # wait for AUTH_OK
    if mtype != 7:
        print(f"[user{idx}] auth failed: type={mtype}")
        s.close()
        exit(1)

    latencies = []
    for j in range(50):
        # broadcast_payload_t: sender[32] + text[1024]
        sender = f"user{idx}".encode().ljust(32, b'\x00')
        text   = f"msg {j} from user{idx}".encode().ljust(1024, b'\x00')
        payload = sender + text

        t0 = time.time_ns()
        send_msg(s, 3, payload)
        # Don't wait for echo — just record send time and move on
        t1 = time.time_ns()
        latencies.append((t1 - t0) // 1_000_000)
        time.sleep(0.05)

    with open(f"$LOGDIR/client_{idx}.csv", "w") as f:
        f.write("latency_ms\n")
        for l in latencies:
            f.write(f"{l}\n")
    print(f"[user{idx}] done, {len(latencies)} messages sent")

    send_msg(s, 6, b"")
    s.close()

except Exception as e:
    print(f"[user{idx}] error: {e}")
PYEOF
    ) &
done

wait
echo "Load test complete. Logs in $LOGDIR/"