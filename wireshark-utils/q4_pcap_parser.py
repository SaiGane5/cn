#!/usr/bin/env python3

import sys
import subprocess
from collections import defaultdict, Counter
import os

class PCAPReader:
    def __init__(self, filename):
        self.filename = filename

    def run_tshark(self, display_filter, fields):
        """Helper to run tshark with specific filters and fields"""
        cmd = ["tshark", "-r", self.filename, "-Y", display_filter, "-T", "fields", "-E", "separator=|"]
        for f in fields:
            cmd.extend(["-e", f])
        
        try:
            result = subprocess.run(cmd, stdout=subprocess.PIPE, text=True, check=True)
            return [line.split('|') for line in result.stdout.splitlines() if line]
        except subprocess.CalledProcessError:
            return []

def analyze_pcap(pcap_file):
    print("=" * 70)
    print("CS 3205 Assignment 1 - Question 4: PCAP Analysis")
    print("=" * 70)
    
    reader = PCAPReader(pcap_file)

    # 1. Identify Server and Clients
    # Find the most frequent destination of GET requests
    all_gets = reader.run_tshark("http.request.method == GET", ["ip.dst", "ip.src"])
    if not all_gets:
        print("No HTTP GET traffic found.")
        return

    server_ip = Counter([row[0] for row in all_gets]).most_common(1)[0][0]
    client_ips = sorted(list(set([row[1] for row in all_gets])))

    print(f"\nQ4(a): Server IP: {server_ip}")
    print(f"Client IPs: {', '.join(client_ips)}")

    # 2. GET requests per client
    print("\n" + "=" * 70)
    print("Q4(b): GET Requests per Client")
    client_counts = Counter([row[1] for row in all_gets])
    for client, count in client_counts.items():
        print(f"  {client}: {count} requests")

    # 3. raw.mp4 Download Times (Using tcp.stream logic)
    print("\n" + "=" * 70)
    print("Q4(c): Download Time of raw.mp4")
    
    # Find streams containing raw.mp4
    mp4_data = reader.run_tshark('http.request.uri contains "raw.mp4"', ["tcp.stream", "frame.time_epoch", "ip.src"])
    
    raw_mp4_results = []
    if mp4_data:
        for stream_id, start_time, client_ip in mp4_data:
            # For this specific stream, find the timestamp of the very last packet
            stream_packets = reader.run_tshark(f"tcp.stream == {stream_id}", ["frame.time_epoch"])
            if stream_packets:
                end_time = float(stream_packets[-1][0])
                duration = end_time - float(start_time)
                raw_mp4_results.append((client_ip, duration))
                print(f"  Client {client_ip}: {duration:.6f} seconds")
    else:
        print("  No raw.mp4 downloads found.")

    # 4. Persistence and Metadata
    print("\n" + "=" * 70)
    print("Q4(e/f): Metadata and Persistence")
    
    meta = reader.run_tshark("http.request or http.response", 
                             ["http.user_agent", "http.response.code", "http.response.phrase", "http.server", "tcp.stream"])
    
    if meta:
        # Sample User Agent from first request
        ua = next((row[0] for row in meta if row[0]), "Unknown")
        print(f"  User-Agent: {ua}")
        
        # Sample Response Code
        resp = next(((row[1], row[2]) for row in meta if row[1]), ("Unknown", ""))
        print(f"  HTTP Response: {resp[0]} {resp[1]}")
        
        # Web Server
        srv = next((row[3] for row in meta if row[3]), "Unknown")
        print(f"  Web Server: {srv}")

        # Persistence check: Count HTTP requests per TCP stream
        stream_ids = [row[4] for row in meta if row[4]]
        requests_per_stream = Counter(stream_ids)
        is_persistent = any(count > 1 for count in requests_per_stream.values())
        print(f"  HTTP Persistence: {'PERSISTENT' if is_persistent else 'NON-PERSISTENT'}")

    # 5. Parallel Connections
    print("\n" + "=" * 70)
    print("Q4(g): Parallel Connections")
    
    # Logic from reference: Count unique streams per client
    all_streams = reader.run_tshark("ip", ["ip.src", "ip.dst", "tcp.stream"])
    client_stream_map = defaultdict(set)
    for src, dst, sid in all_streams:
        if src in client_ips:
            client_stream_map[src].add(sid)
        elif dst in client_ips:
            client_stream_map[dst].add(sid)

    for client, streams in client_stream_map.items():
        print(f"  Client {client}: {len(streams)} unique connections")

if __name__ == "__main__":
    os.makedirs('q4_outputs', exist_ok=True)
    pcap = "Online-quiz-classroom.pcap"
    if os.path.exists(pcap):
        analyze_pcap(pcap)
    else:
        print(f"Error: {pcap} not found.")