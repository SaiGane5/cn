#!/usr/bin/env python3

"""
CS 3205 Assignment 1 - Question 4(d)
Plot download times for raw.mp4
"""

import sys

# Read the data
data = []
with open('q4_outputs/download_times_data.txt', 'r') as f:
    f.readline()  # Skip header
    for line in f:
        if line.strip():
            ip, time = line.strip().split(',')
            data.append((ip, float(time)))

# Sort by IP for consistent ordering
data.sort(key=lambda x: x[0])

# Create simple bar chart using ASCII
print("\n" + "=" * 80)
print("Download Time of raw.mp4 by Client")
print("=" * 80)
print()

max_time = max(t for _, t in data)
scale = 60 / max_time  # Scale to 60 characters width

client_id = 1
for ip, time in data:
    bar_length = int(time * scale)
    bar = '█' * bar_length
    print(f"C{client_id:2d} ({ip:15s}): {bar} {time:.2f}s")
    client_id += 1

print()
print("=" * 80)
print()

# Try to create actual plot if matplotlib is available
try:
    import matplotlib.pyplot as plt
    
    clients = [f"C{i+1}" for i in range(len(data))]
    times = [t for _, t in data]
    
    plt.figure(figsize=(12, 6))
    bars = plt.bar(clients, times, color='steelblue', edgecolor='navy', linewidth=1.5)
    
    # Color code by time
    for i, (bar, time) in enumerate(zip(bars, times)):
        if time > 170:
            bar.set_color('#e74c3c')  # Red for slow
        elif time > 160:
            bar.set_color('#f39c12')  # Orange for medium
        else:
            bar.set_color('#27ae60')  # Green for fast
    
    plt.xlabel('Client ID', fontsize=12, fontweight='bold')
    plt.ylabel('Download Time (seconds)', fontsize=12, fontweight='bold')
    plt.title('raw.mp4 Download Time by Client', fontsize=14, fontweight='bold')
    plt.grid(axis='y', alpha=0.3, linestyle='--')
    
    # Add value labels on bars
    for i, (client, time) in enumerate(zip(clients, times)):
        plt.text(i, time + 1, f'{time:.1f}s', ha='center', va='bottom', fontsize=9)
    
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig('q4_outputs/download_times_plot.png', dpi=300, bbox_inches='tight')
    print("Plot saved to q4_outputs/download_times_plot.png")
    
    # Save client mapping
    with open('q4_outputs/client_id_mapping.txt', 'w') as f:
        f.write("Client ID to IP Address Mapping:\n")
        f.write("=" * 40 + "\n")
        for i, (ip, time) in enumerate(data):
            f.write(f"C{i+1:2d} = {ip:15s} ({time:.2f}s)\n")
    
    print("Client mapping saved to q4_outputs/client_id_mapping.txt")
    
except ImportError:
    print("Note: matplotlib not available for graphical plot")
    print("ASCII chart shown above, or create chart in Excel using download_times_data.txt")

print()
