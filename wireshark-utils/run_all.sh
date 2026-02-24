#!/bin/bash

# CS 3205 Assignment 1 - Master Execution Script
# Run this to execute all questions (except Q5 which needs manual capture)

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║         CS 3205 Assignment 1 - Master Script                  ║"
echo "║         Network Analysis on macOS                              ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Check if tools are installed
echo "Checking required tools..."
MISSING_TOOLS=0

if ! command -v tcptraceroute &> /dev/null; then
    echo "❌ tcptraceroute not found"
    MISSING_TOOLS=1
else
    echo "✓ tcptraceroute found"
fi

if ! command -v tcping &> /dev/null; then
    echo "❌ tcping not found"
    MISSING_TOOLS=1
else
    echo "✓ tcpping found"
fi

if [ $MISSING_TOOLS -eq 1 ]; then
    echo ""
    echo "⚠️  Missing tools detected!"
    echo "Please run: ./install_tools.sh"
    echo ""
    read -p "Do you want to continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo ""
echo "════════════════════════════════════════════════════════════════"
echo "QUESTION 1: Traceroute and Ping Analysis"
echo "════════════════════════════════════════════════════════════════"
echo ""
read -p "Run Question 1? This will take ~10-15 minutes. (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    chmod +x q1_traceroute_ping.sh
    ./q1_traceroute_ping.sh
    echo ""
    echo "Analyzing results..."
    python3 q1_analysis.py
    echo ""
    echo "✓ Question 1 complete! Results in q1_outputs/"
else
    echo "⊘ Skipped Question 1"
fi

echo ""
echo "════════════════════════════════════════════════════════════════"
echo "QUESTION 2: DNS Analysis"
echo "════════════════════════════════════════════════════════════════"
echo ""
read -p "Run Question 2? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    chmod +x q2_dns_analysis.sh
    ./q2_dns_analysis.sh
    echo ""
    echo "✓ Question 2 complete! Results in q2_outputs/"
    echo ""
    echo "NOTE: You still need to manually traceroute to the DNS servers shown above"
else
    echo "⊘ Skipped Question 2"
fi

echo ""
echo "════════════════════════════════════════════════════════════════"
echo "QUESTION 3: Localhost Ping Failure"
echo "════════════════════════════════════════════════════════════════"
echo ""
read -p "Run Question 3? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    chmod +x q3_localhost_fail.sh
    ./q3_localhost_fail.sh
    echo ""
    echo "✓ Question 3 instructions created in q3_outputs/"
    echo ""
    echo "To make ping fail, run:"
    echo "  sudo pfctl -e -f q3_outputs/pf_block.conf"
    echo "Then test: ping -c 5 127.0.0.1"
    echo "To restore: sudo pfctl -d"
else
    echo "⊘ Skipped Question 3"
fi

echo ""
echo "════════════════════════════════════════════════════════════════"
echo "QUESTION 4: PCAP Analysis"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "Running Q4 analysis (already have PCAP file)..."
python3 q4_pcap_parser.py
echo ""
python3 q4_plot_downloads.py
echo ""
echo "✓ Question 4 complete! Results in q4_outputs/"

echo ""
echo "════════════════════════════════════════════════════════════════"
echo "QUESTION 5: YouTube Streaming Analysis"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "Question 5 requires manual packet capture."
echo ""
echo "Instructions:"
echo "1. Find your network interface: route get google.com | grep interface"
echo "2. Start capture: sudo tcpdump -i en0 -w q5_outputs/youtube_capture.pcap 'host googlevideo.com or host youtube.com'"
echo "3. Play YouTube video for 30 seconds"
echo "4. Stop capture (Ctrl+C)"
echo "5. Run: python3 q5_youtube_analysis.py"
echo ""
echo "See README.md for detailed instructions"

echo ""
echo "════════════════════════════════════════════════════════════════"
echo "All automated tasks complete!"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "Summary:"
echo "  Q1: Outputs in q1_outputs/"
echo "  Q2: Outputs in q2_outputs/"
echo "  Q3: Instructions in q3_outputs/"
echo "  Q4: Outputs in q4_outputs/ (including plot!)"
echo "  Q5: Manual capture required - see README.md"
echo ""
echo "Next steps:"
echo "1. Review all outputs and take screenshots"
echo "2. Complete Q5 manually"
echo "3. Create your report (see report_template.md)"
echo "4. Package everything as <Roll_No>-assignment1.tar.gz"
echo ""
echo "Good luck! 🎓"
