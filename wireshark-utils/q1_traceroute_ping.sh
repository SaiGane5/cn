#!/bin/bash

# CS 3205 Assignment 1 - Question 1
# Traceroute and Ping Analysis

OUTPUT_DIR="q1_outputs"
mkdir -p "$OUTPUT_DIR"

echo "======================================================"
echo "Question 1: Traceroute and Ping Analysis"
echo "======================================================"

# Q1(a) - Traceroute to IITB
echo ""
echo "Q1(a): Running traceroute to iitb.ac.in..."
sudo tcptraceroute iitb.ac.in 80 > "$OUTPUT_DIR/traceroute_iitb.txt"
cat "$OUTPUT_DIR/traceroute_iitb.txt"

echo ""
echo "Pausing for 5 seconds..."
sleep 5

# Q1(a) - Traceroute to AUST
echo ""
echo "Running traceroute to aust.edu.ng..."
sudo tcptraceroute aust.edu.ng 80 > "$OUTPUT_DIR/traceroute_aust.txt"
cat "$OUTPUT_DIR/traceroute_aust.txt"

echo ""
echo "Pausing for 5 seconds..."
sleep 5

# Q1(c) - Ping IITB 100 times
echo ""
echo "Q1(c): Pinging iitb.ac.in 100 times..."
tcping -c 100 iitb.ac.in 80 > "$OUTPUT_DIR/ping_iitb.txt"
cat "$OUTPUT_DIR/ping_iitb.txt"

echo ""
echo "Pausing for 5 seconds..."
sleep 5

# Q1(d) - Ping AUST 100 times
echo ""
echo "Q1(d): Pinging aust.edu.ng 100 times..."
tcpping -c 100 aust.edu.ng 80 > "$OUTPUT_DIR/ping_aust.txt"
cat "$OUTPUT_DIR/ping_aust.txt"

echo ""
echo "======================================================"
echo "All Q1 outputs saved in $OUTPUT_DIR/"
echo "======================================================"
