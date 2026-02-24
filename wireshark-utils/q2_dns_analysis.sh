#!/bin/bash

# CS 3205 Assignment 1 - Question 2
# DNS Server Analysis

OUTPUT_DIR="q2_outputs"
mkdir -p "$OUTPUT_DIR"

echo "======================================================"
echo "Question 2: DNS Server Analysis"
echo "======================================================"

# Q2(a) - Find DNS server for IITB
echo ""
echo "Q2(a): Finding DNS server for iitb.ac.in..."
nslookup -type=NS iitb.ac.in > "$OUTPUT_DIR/dns_iitb.txt"
cat "$OUTPUT_DIR/dns_iitb.txt"

echo ""
echo "Getting authoritative DNS server IP..."
dig iitb.ac.in NS +short > "$OUTPUT_DIR/dns_iitb_servers.txt"
cat "$OUTPUT_DIR/dns_iitb_servers.txt"

echo ""
echo "Pausing for 3 seconds..."
sleep 3

# Find DNS server for AUST
echo ""
echo "Finding DNS server for aust.edu.ng..."
nslookup -type=NS aust.edu.ng > "$OUTPUT_DIR/dns_aust.txt"
cat "$OUTPUT_DIR/dns_aust.txt"

echo ""
echo "Getting authoritative DNS server IP..."
dig aust.edu.ng NS +short > "$OUTPUT_DIR/dns_aust_servers.txt"
cat "$OUTPUT_DIR/dns_aust_servers.txt"

echo ""
echo "Pausing for 3 seconds..."
sleep 3

# Get local DNS server (resolver)
echo ""
echo "Finding YOUR local DNS server (resolver)..."
scutil --dns | grep "nameserver" | head -5 > "$OUTPUT_DIR/local_dns.txt"
cat "$OUTPUT_DIR/local_dns.txt"

echo ""
echo "Pausing for 3 seconds..."
sleep 3

# Q2(b) - Traceroute to DNS servers
echo ""
echo "Q2(b): Traceroute to web servers (from Q1)..."
echo "Traceroute to iitb.ac.in web server:"
tcptraceroute iitb.ac.in 80 > "$OUTPUT_DIR/traceroute_iitb_web.txt"
cat "$OUTPUT_DIR/traceroute_iitb_web.txt" | tail -5

echo ""
echo "Pausing for 5 seconds..."
sleep 5

echo ""
echo "Traceroute to aust.edu.ng web server:"
tcptraceroute aust.edu.ng 80 > "$OUTPUT_DIR/traceroute_aust_web.txt"
cat "$OUTPUT_DIR/traceroute_aust_web.txt" | tail -5

echo ""
echo "======================================================"
echo "DNS server information saved in $OUTPUT_DIR/"
echo "======================================================"
echo ""
echo "NOTE: You'll need to manually traceroute to the authoritative"
echo "DNS servers shown above to compare hop counts."
echo "Authoritative DNS servers are usually different from local resolvers."
