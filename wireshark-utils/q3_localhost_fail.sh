#!/bin/bash

# CS 3205 Assignment 1 - Question 3
# Make ping fail to 127.0.0.1

OUTPUT_DIR="q3_outputs"
mkdir -p "$OUTPUT_DIR"

echo "======================================================"
echo "Question 3: Make ping fail to 127.0.0.1"
echo "======================================================"
echo ""
echo "We'll use firewall rules to block ICMP/TCP to localhost"
echo ""

# First, try normal ping to show it works
echo "Step 1: Normal ping to 127.0.0.1 (should work):"
ping -c 5 127.0.0.1 > "$OUTPUT_DIR/ping_before.txt" 2>&1
cat "$OUTPUT_DIR/ping_before.txt"

echo ""
echo "------------------------------------------------------"
echo ""

# Method 1: Using pfctl (macOS firewall)
echo "Step 2: Creating firewall rule to block localhost traffic..."
echo ""
echo "Creating pf.conf rule file..."

cat > "$OUTPUT_DIR/pf_block.conf" << 'EOF'
# Block all traffic to and from 127.0.0.1
block drop quick on lo0 from any to 127.0.0.1
block drop quick on lo0 from 127.0.0.1 to any
EOF

echo "Firewall rule created. Content:"
cat "$OUTPUT_DIR/pf_block.conf"

echo ""
echo "------------------------------------------------------"
echo ""
echo "To apply this rule and make ping fail, run these commands:"
echo ""
echo "  sudo pfctl -e -f q3_outputs/pf_block.conf"
echo ""
echo "Then test with:"
echo "  ping -c 5 127.0.0.1"
echo "  tcpping -c 5 127.0.0.1 80"
echo ""
echo "To restore normal operation, run:"
echo "  sudo pfctl -d"
echo ""
echo "------------------------------------------------------"
echo ""
echo "ALTERNATIVE METHOD (Manual):"
echo "You can also add a route to blackhole:"
echo "  sudo route add 127.0.0.1 127.0.0.2"
echo ""
echo "To remove it:"
echo "  sudo route delete 127.0.0.1"
echo ""
echo "------------------------------------------------------"

# Save instructions to file
cat > "$OUTPUT_DIR/INSTRUCTIONS.txt" << 'EOF'
CS 3205 Assignment 1 - Question 3
How to make ping fail to 127.0.0.1

METHOD 1: Using Packet Filter (pfctl) - RECOMMENDED
====================================================

1. Enable and load the firewall rule:
   sudo pfctl -e -f q3_outputs/pf_block.conf

2. Test that ping fails:
   ping -c 5 127.0.0.1
   
   Expected: 100% packet loss

3. Take screenshot showing the failed ping

4. Restore normal operation:
   sudo pfctl -d


METHOD 2: Using Route Blackhole
====================================================

1. Add blackhole route:
   sudo route add 127.0.0.1 127.0.0.2

2. Test that ping fails:
   ping -c 5 127.0.0.1

3. Take screenshot showing the failed ping

4. Restore normal operation:
   sudo route delete 127.0.0.1


EXPLANATION:
============

Method 1 works by:
- Using macOS's packet filter (pf) firewall
- Blocking all packets to/from 127.0.0.1 on the loopback interface
- This causes 100% packet loss as packets are dropped before reaching the network stack

Method 2 works by:
- Adding a route that redirects 127.0.0.1 to 127.0.0.2 (non-existent)
- This causes packets to be sent to the wrong destination
- Results in 100% packet loss due to unreachable destination
EOF

cat "$OUTPUT_DIR/INSTRUCTIONS.txt"

echo ""
echo "======================================================"
echo "Instructions saved to $OUTPUT_DIR/INSTRUCTIONS.txt"
echo "======================================================"
