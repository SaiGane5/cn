#!/usr/bin/env python3

"""
CS 3205 Assignment 1 - Question 1 Analysis
Analyze traceroute and ping data
"""

import re
import statistics
from math import radians, sin, cos, sqrt, atan2

def parse_traceroute(filename):
    """Parse traceroute output and extract hop information"""
    hops = []
    
    with open(filename, 'r') as f:
        content = f.read()
    
    lines = content.strip().split('\n')
    
    for line in lines:
        # Skip header lines
        if 'traceroute' in line.lower() or not line.strip():
            continue
        
        # Parse hop line: " 1  192.168.1.1  1.234 ms  1.123 ms  1.456 ms"
        # or with hostname: " 1  router.local (192.168.1.1)  1.234 ms  1.123 ms  1.456 ms"
        match = re.match(r'\s*(\d+)\s+(.+)', line)
        if match:
            hop_num = int(match.group(1))
            rest = match.group(2)
            
            # Check for * * * (unreachable hop)
            if '***' in rest or '* * *' in rest:
                continue
            
            # Extract IP address
            ip_match = re.search(r'(\d+\.\d+\.\d+\.\d+)', rest)
            if ip_match:
                ip_addr = ip_match.group(1)
                
                # Extract latencies (in ms)
                latencies = re.findall(r'(\d+\.?\d*)\s*ms', rest)
                latencies = [float(lat) for lat in latencies]
                
                if latencies:
                    avg_latency = statistics.mean(latencies)
                    hops.append({
                        'hop': hop_num,
                        'ip': ip_addr,
                        'latencies': latencies,
                        'avg_latency': avg_latency
                    })
    
    return hops

def parse_ping_summary(filename):
    """Parse nping output and extract average latency"""
    with open(filename, 'r') as f:
        content = f.read()
    
    # New regex to match nping format: Max rtt: 71.262ms | Min rtt: 41.145ms | Avg rtt: 43.672ms
    match = re.search(r'Max rtt:\s*([\d.]+)ms\s*\|\s*Min rtt:\s*([\d.]+)ms\s*\|\s*Avg rtt:\s*([\d.]+)ms', content)
    
    if match:
        return {
            'max': float(match.group(1)),
            'min': float(match.group(2)),
            'avg': float(match.group(3)),
            'stddev': 0  # nping summary doesn't always provide stddev
        }
    return None
    
    # Alternative: parse individual ping times
    ping_times = re.findall(r'time[=\s]+([\d.]+)\s*ms', content)
    if ping_times:
        ping_times = [float(t) for t in ping_times]
        return {
            'min': min(ping_times),
            'avg': statistics.mean(ping_times),
            'max': max(ping_times),
            'stddev': statistics.stdev(ping_times) if len(ping_times) > 1 else 0
        }
    
    return None

def haversine_distance(lat1, lon1, lat2, lon2):
    """Calculate great circle distance between two points on Earth (in km)"""
    R = 6371  # Earth's radius in kilometers
    
    lat1, lon1, lat2, lon2 = map(radians, [lat1, lon1, lat2, lon2])
    dlat = lat2 - lat1
    dlon = lon2 - lon1
    
    a = sin(dlat/2)**2 + cos(lat1) * cos(lat2) * sin(dlon/2)**2
    c = 2 * atan2(sqrt(a), sqrt(1-a))
    distance = R * c
    
    return distance

def calculate_expected_latency(distance_km):
    """Calculate expected latency based on speed of light"""
    # Speed of light in fiber = ~2/3 * c = 2 * 10^8 m/s
    speed_light_fiber = 3e8  # m/s
    distance_m = distance_km * 1000
    
    # Round trip time (RTT)
    rtt_seconds = (2 * distance_m) / speed_light_fiber
    rtt_ms = rtt_seconds * 1000
    
    return rtt_ms

def main():
    print("=" * 70)
    print("CS 3205 Assignment 1 - Question 1 Analysis")
    print("=" * 70)
    
    # Parse traceroute data
    print("\n--- Q1(a): Traceroute Analysis ---\n")
    
    print("IITB Traceroute:")
    iitb_hops = parse_traceroute('q1_outputs/traceroute_iitb.txt')
    for hop in iitb_hops:
        print(f"  Hop {hop['hop']}: {hop['ip']} - Avg Latency: {hop['avg_latency']:.3f} ms")
    
    print(f"\nTotal visible hops to IITB: {len(iitb_hops)}")
    
    print("\n" + "-" * 70)
    print("\nAUST Traceroute:")
    aust_hops = parse_traceroute('q1_outputs/traceroute_aust.txt')
    for hop in aust_hops:
        print(f"  Hop {hop['hop']}: {hop['ip']} - Avg Latency: {hop['avg_latency']:.3f} ms")
    
    print(f"\nTotal visible hops to AUST: {len(aust_hops)}")
    
    # Q1(b): Compare hops
    print("\n--- Q1(b): Comparison ---\n")
    print(f"Number of hops to IITB: {len(iitb_hops)}")
    print(f"Number of hops to AUST: {len(aust_hops)}")
    print(f"Difference: {abs(len(iitb_hops) - len(aust_hops))} hops")
    
    # Parse ping data
    print("\n--- Q1(c) & Q1(d): Ping Analysis ---\n")
    
    iitb_ping = parse_ping_summary('q1_outputs/ping_iitb.txt')
    if iitb_ping:
        print(f"IITB Ping (100 packets):")
        print(f"  Min: {iitb_ping['min']:.3f} ms")
        print(f"  Avg: {iitb_ping['avg']:.3f} ms")
        print(f"  Max: {iitb_ping['max']:.3f} ms")
        print(f"  StdDev: {iitb_ping['stddev']:.3f} ms")
    
    print()
    
    aust_ping = parse_ping_summary('q1_outputs/ping_aust.txt')
    if aust_ping:
        print(f"AUST Ping (100 packets):")
        print(f"  Min: {aust_ping['min']:.3f} ms")
        print(f"  Avg: {aust_ping['avg']:.3f} ms")
        print(f"  Max: {aust_ping['max']:.3f} ms")
        print(f"  StdDev: {aust_ping['stddev']:.3f} ms")
    
    # Q1(e): Expected latency calculation
    print("\n--- Q1(e): Expected Latency (Speed of Light) ---\n")
    
    
    iitb_distance = 1317  # km (Chennai to Mumbai, approximate)
    aust_distance = 3750  # km (Chennai to Abuja, approximate)
    
    iitb_expected = calculate_expected_latency(iitb_distance)
    aust_expected = calculate_expected_latency(aust_distance)
    
    print(f"IITB (distance ~{iitb_distance} km):")
    print(f"  Expected latency: {iitb_expected:.3f} ms")
    if iitb_ping:
        print(f"  Actual latency: {iitb_ping['avg']:.3f} ms")
        print(f"  Difference: {iitb_ping['avg'] - iitb_expected:.3f} ms")
    
    print(f"\nAUST (distance ~{aust_distance} km):")
    print(f"  Expected latency: {aust_expected:.3f} ms")
    if aust_ping:
        print(f"  Actual latency: {aust_ping['avg']:.3f} ms")
        print(f"  Difference: {aust_ping['avg'] - aust_expected:.3f} ms")
    
    # Q1(f): Sum of intermediate hop latencies
    print("\n--- Q1(f): Sum of Intermediate Hop Latencies ---\n")
    
    if iitb_hops:
        iitb_sum = sum(hop['avg_latency'] for hop in iitb_hops)
        print(f"IITB - Sum of all hop latencies: {iitb_sum:.3f} ms")
        if iitb_ping:
            print(f"IITB - Direct ping latency: {iitb_ping['avg']:.3f} ms")
            print(f"IITB - Difference: {abs(iitb_sum - iitb_ping['avg']):.3f} ms")
    
    print()
    
    if aust_hops:
        aust_sum = sum(hop['avg_latency'] for hop in aust_hops)
        print(f"AUST - Sum of all hop latencies: {aust_sum:.3f} ms")
        if aust_ping:
            print(f"AUST - Direct ping latency: {aust_ping['avg']:.3f} ms")
            print(f"AUST - Difference: {abs(aust_sum - aust_ping['avg']):.3f} ms")
    
    # Q1(g): Maximum hop latency
    print("\n--- Q1(g): Maximum Hop Latency ---\n")
    
    if iitb_hops:
        iitb_max_hop = max(iitb_hops, key=lambda x: x['avg_latency'])
        print(f"IITB - Max hop latency: {iitb_max_hop['avg_latency']:.3f} ms (Hop {iitb_max_hop['hop']})")
        if iitb_ping:
            print(f"IITB - Direct ping latency: {iitb_ping['avg']:.3f} ms")
    
    print()
    
    if aust_hops:
        aust_max_hop = max(aust_hops, key=lambda x: x['avg_latency'])
        print(f"AUST - Max hop latency: {aust_max_hop['avg_latency']:.3f} ms (Hop {aust_max_hop['hop']})")
        if aust_ping:
            print(f"AUST - Direct ping latency: {aust_ping['avg']:.3f} ms")
    
    print("\n" + "=" * 70)
    print("Analysis complete! Review the results above for your report.")
    print("=" * 70)

if __name__ == "__main__":
    main()
