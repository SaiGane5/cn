#!/bin/bash

echo "Installing required tools for CS 3205 Assignment 1..."
echo "======================================================="

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Homebrew not found. Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
    echo "✓ Homebrew is already installed"
fi

# Install tcptraceroute
if ! command -v tcptraceroute &> /dev/null; then
    echo "Installing tcptraceroute..."
    brew install tcptraceroute
else
    echo "✓ tcptraceroute is already installed"
fi

# Install tcpping
if ! command -v tcping &> /dev/null; then
    echo "Installing tcping..."
    brew install tcping
else
    echo "✓ tcping is already installed"
fi

# Install Wireshark/tshark
if ! command -v tshark &> /dev/null; then
    echo "Installing Wireshark (includes tshark)..."
    brew install --cask wireshark
else
    echo "✓ tshark is already installed"
fi

# Install Python packages
python3 -m venv .venv
source .venv/bin/activate
echo "Installing Python packages..."
pip3 install scapy matplotlib pandas numpy --break-system-packages

echo ""
echo "======================================================="
echo "Installation complete!"
echo "======================================================="
echo ""
echo "Please run the following commands to verify installation:"
echo "  tcptraceroute --version"
echo "  tcping -h"
echo "  tshark --version"
echo "  python3 -c 'import scapy; print(\"Scapy OK\")'"
