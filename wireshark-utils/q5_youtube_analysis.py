#!/usr/bin/env python3

import subprocess
import os
import matplotlib.pyplot as plt
from collections import defaultdict

PCAP = 'youtube_capture.pcap'

def run_tshark(cmd):
    """Helper to run tshark commands and return results as a list of lines"""
    try:
        return subprocess.check_output(cmd).decode().strip().splitlines()
    except subprocess.CalledProcessError:
        return []

def analyze_youtube():
    if not os.path.exists(PCAP):
        print(f"Error: {PCAP} not found. Please ensure the capture file is in this directory.")
        return

    # --- 1. Dynamic Server Detection ---
    # Automatically finds the IP sending the most data (the YouTube video server)
    ip_cmd = ["tshark", "-r", PCAP, "-T", "fields", "-e", "ip.src", "-c", "2000"]
    ips = run_tshark(ip_cmd)
    if not ips: 
        print("No traffic found in the PCAP file.")
        return
    server_ip = max(set(ips), key=ips.count)

    # --- Q5(a): Throughput Analysis ---
    print("\n" + "="*80)
    print(f"Q5(a): THROUGHPUT ANALYSIS (Server: {server_ip})")
    print("="*80)

    throughput_cmd = [
        "tshark", "-r", PCAP,
        "-Y", f"ip.src == {server_ip}",
        "-T", "fields", "-e", "frame.time_epoch", "-e", "frame.len"
    ]

    throughput_data = defaultdict(float)
    start_time = None
    
    for line in run_tshark(throughput_cmd):
        parts = line.split('\t')
        if len(parts) == 2:
            timestamp, length = float(parts[0]), int(parts[1])
            if start_time is None: start_time = timestamp
            
            second = int(timestamp - start_time)
            # Mbps formula: (Bytes * 8 bits/byte) / 1,000,000
            throughput_data[second] += (length * 8) / 1_000_000

    seconds = sorted(throughput_data.keys())
    mbps_values = [throughput_data[s] for s in seconds]

    if mbps_values:
        print(f"Peak Throughput:    {max(mbps_values):.3f} Mbps")
        print(f"Average Throughput: {sum(mbps_values)/len(mbps_values):.3f} Mbps")
        
        # Plotting
        plt.figure(figsize=(10, 5))
        plt.plot(seconds, mbps_values, color='red', marker='o', markersize=4, linestyle='-')
        plt.title(f"YouTube Throughput Over Time ({server_ip})")
        plt.xlabel("Time (seconds)")
        plt.ylabel("Throughput (Mbps)")
        plt.grid(True, linestyle='--', alpha=0.6)
        plt.savefig('throughput_plot.png')
        print("Plot saved as 'throughput_plot.png'. Displaying window...")
        plt.show() 
    else:
        print("No data found for throughput calculation.")

    # --- Q5(c): TCP Connection States ---
    print("\n" + "="*80)
    print("Q5(c): TCP CONNECTION STATES")
    print(f"{'STREAM ID':<10} | {'SERVER IP':<20} | {'CLIENT IP':<20} | {'STATE'}")
    print("-" * 80)

    # Extract TCP stream index, source IP, dest IP, and TCP flags
    state_cmd = [
        "tshark", "-r", PCAP, "-Y", "tcp",
        "-T", "fields", "-e", "tcp.stream", "-e", "ip.src", "-e", "ip.dst", "-e", "tcp.flags"
    ]

    streams = defaultdict(lambda: {'flags': set(), 'src': '', 'dst': ''})
    for line in run_tshark(state_cmd):
        parts = line.split('\t')
        if len(parts) == 4:
            sid, src, dst, f_hex = parts[0], parts[1], parts[2], int(parts[3], 16)
            streams[sid]['flags'].add(f_hex)
            # Identify which IP is the server vs the local machine
            if src == server_ip:
                streams[sid]['src'], streams[sid]['dst'] = src, dst
            else:
                streams[sid]['src'], streams[sid]['dst'] = dst, src

    for sid in sorted(streams.keys(), key=int):
        flags = streams[sid]['flags']
        
        # Determine state using flag analysis
        is_reset = any(f & 0x04 for f in flags)  # RST
        is_fin   = any(f & 0x01 for f in flags)  # FIN
        is_syn   = any(f & 0x02 for f in flags)  # SYN
        is_ack   = any(f & 0x10 for f in flags)  # ACK
        
        if is_reset:
            state = "RESET (Aborted)"
        elif is_fin:
            state = "CLOSED (Graceful)"
        elif is_syn and is_ack:
            state = "ESTABLISHED"
        else:
            state = "DATA TRANSFER"

        # Display output for each identified stream
        print(f"{sid:<10} | {streams[sid]['src']:<20} | {streams[sid]['dst']:<20} | {state}")
    
    print("="*80)
    print("Analysis Complete.")

if __name__ == "__main__":
    analyze_youtube()