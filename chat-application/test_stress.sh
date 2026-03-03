#!/bin/bash
# Stress test: gradually increase clients 1..10, 20 msgs each
PORT=8080
LOGDIR="stress_logs"
mkdir -p $LOGDIR

for NUM_CLIENTS in 1 2 4 6 8 10; do
    echo "=== $NUM_CLIENTS clients ==="
    for i in $(seq 1 $NUM_CLIENTS); do
        (
            python3 - <<PYEOF
import socket, struct, time

def send_msg(fd, mtype, payload):
    data = payload if isinstance(payload, bytes) else payload.encode()
    fd.send(struct.pack("!BL", mtype, len(data)) + data)

s = socket.socket()
s.connect(("127.0.0.1", $PORT))
uname = f"stress$i".encode().ljust(32, b'\x00')
passwd = b"pass".ljust(32, b'\x00')
send_msg(s, 2, uname + passwd)
s.recv(64)

for j in range(20):
    payload = b'\x00' * 32 + b'\x00' * 32 + f"stress msg {j}".encode().ljust(1024, b'\x00')
    send_msg(s, 3, payload)
    time.sleep(0.1)

send_msg(s, 6, b"")
s.close()
PYEOF
        ) &
    done
    wait
    # capture a snapshot of the server metrics at this load
    echo "$NUM_CLIENTS" >> $LOGDIR/load_levels.txt
    sleep 5
done
echo "Stress test done."