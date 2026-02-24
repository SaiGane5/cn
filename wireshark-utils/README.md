# CS 3205 Assignment 1 - Complete Guide
# Network Analysis Assignment
# Student Instructions for macOS

## 📋 Overview

This package contains all scripts and tools needed to complete CS 3205 Assignment 1 on macOS.

## 🚀 Quick Start

### Step 1: Install Required Tools

```bash
chmod +x install_tools.sh
./install_tools.sh
```

This will install:
- tcptraceroute (for TCP-based traceroute)
- tcpping (for TCP-based ping)
- Wireshark/tshark (for packet analysis)
- Python packages (matplotlib, etc.)

**Note:** You may need to grant network permissions to these tools in macOS System Preferences > Security & Privacy.

---

## 📝 Assignment Questions & How to Run

### **QUESTION 1: Traceroute and Ping Analysis**

#### Run the data collection:

```bash
chmod +x q1_traceroute_ping.sh
./q1_traceroute_ping.sh
```

This will:
- Traceroute to iitb.ac.in and aust.edu.ng
- Ping both destinations 100 times
- Save all outputs to `q1_outputs/` directory

#### Analyze the results:

```bash
python3 q1_analysis.py
```

This will provide analysis for all parts (a) through (g).

#### **What to include in your report:**

**Q1(a):** Number of hops, IP addresses, average latencies
- Screenshot: `q1_outputs/traceroute_iitb.txt` and `traceroute_aust.txt`
- Include the analysis output from q1_analysis.py

**Q1(b):** Comparison of hop counts
- Explain: Geographic distance, routing policies, network infrastructure

**Q1(c) & (d):** Average latency from ping
- Screenshot: `q1_outputs/ping_iitb.txt` and `ping_aust.txt`

**Q1(e):** Expected vs Actual latency
- The script calculates expected latency based on speed of light in fiber (2×10⁸ m/s)
- Actual latency is higher due to: routing delays, queuing delays, processing delays, retransmissions

**Q1(f):** Sum of hop latencies vs direct ping
- Should be similar but not exact
- Explain: Traceroute measures each hop individually at different times; network conditions vary

**Q1(g):** Maximum hop latency vs direct ping
- Maximum hop latency is usually less than direct ping
- Explain: Direct ping includes cumulative delay of entire path

---

### **QUESTION 2: DNS Server Analysis**

#### Run the DNS analysis:

```bash
chmod +x q2_dns_analysis.sh
./q2_dns_analysis.sh
```

#### Manual steps needed:

After running the script, you'll see the authoritative DNS servers. You need to manually traceroute to them:

```bash
# Example (replace with actual DNS server IPs from output)
tcptraceroute dns1.iitb.ac.in 53
tcptraceroute dns1.aust.edu.ng 53
```

#### **What to include in your report:**

**Q2(a):** DNS server IP addresses
- Your local DNS resolver (from `scutil --dns`)
- Authoritative DNS servers for iitb.ac.in and aust.edu.ng
- These are typically different

**Q2(b):** Hop count comparison
- Compare hops to DNS server vs web server
- DNS servers are often closer (Local Name Servers) or may be at the organization
- Explain whether it's likely a Local Name Server or Root Name Server based on proximity

---

### **QUESTION 3: Make Ping Fail to Localhost**

#### Run the script:

```bash
chmod +x q3_localhost_fail.sh
./q3_localhost_fail.sh
```

#### Apply the firewall rule:

```bash
sudo pfctl -e -f q3_outputs/pf_block.conf
```

#### Test that ping fails:

```bash
ping -c 5 127.0.0.1
# Should show 100% packet loss
```

#### **Take screenshot** showing the failed ping

#### Restore normal operation:

```bash
sudo pfctl -d
```

#### **What to include in your report:**

- Screenshot showing 100% packet loss to 127.0.0.1
- Explanation: Using macOS packet filter (pf) to drop all packets to/from loopback interface
- Alternative method: Route blackhole (`sudo route add 127.0.0.1 127.0.0.2`)

---

### **QUESTION 4: PCAP Analysis** ✅ (Already Complete!)

The PCAP file has already been analyzed!

#### View the results:

```bash
python3 q4_pcap_parser.py
python3 q4_plot_downloads.py
```

Results are in `q4_outputs/`:
- `client_ips.txt` - All client IPs
- `server_ips.txt` - All server IPs
- `download_times_data.txt` - Raw data for downloads
- `download_times_plot.png` - Bar chart of download times
- `client_id_mapping.txt` - Client ID to IP mapping

#### **Answers:**

**Q4(a):** Clients and Servers
- **Servers:** 10.129.23.153, 10.129.5.192, 10.129.50.6, 108.160.162.112, 108.160.162.51
- **Clients:** 15 clients (10.129.26.103, 10.129.26.154, etc.)

**Q4(b):** GET requests per client
- Range from 1 to 13 GET requests per client

**Q4(c):** Download time of raw.mp4
- Range from 129.25 to 176.69 seconds
- See detailed list in output

**Q4(d):** Plot
- Graph saved: `q4_outputs/download_times_plot.png`

**Q4(e):** User Agent and Server Info
- **User-Agent:** Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:18.0) Gecko/20100101 Firefox/18.0
- **HTTP Response Code:** 307
- **Response Description:** Temporary Redirect
- **Web Server:** Apache/2.2.22 (Fedora)

**Q4(f):** HTTP Persistence
- **PERSISTENT** - Found 25 connections with multiple requests (connection reuse detected)

**Q4(g):** Parallel Connections
- Browsers opened 6-9 parallel connections per client
- Most clients used 7 parallel connections

---

### **QUESTION 5: YouTube Streaming Analysis**

#### Step 1: Find your active network interface:

```bash
route get google.com | grep interface
# OR
ifconfig | grep -E "^en"
```

Common interfaces:
- `en0` - Usually Wi-Fi
- `en1` - Usually Ethernet

#### Step 2: Start packet capture:

```bash
# Replace 'en0' with your interface
sudo tcpdump -i en0 -w q5_outputs/youtube_capture.pcap 'host googlevideo.com or host youtube.com'
```

#### Step 3: Play YouTube video for 30 seconds

In another terminal/browser:
1. Go to YouTube
2. Play ANY video
3. Let it play for exactly 30 seconds
4. Stop the video

#### Step 4: Stop the capture

Press `Ctrl+C` in the tcpdump terminal

#### Step 5: Analyze the capture:

```bash
python3 q5_youtube_analysis.py
```

#### Step 6: Enable YouTube Stats for Nerds:

1. Go to YouTube Settings > General
2. Enable "Enable Stats for nerds"
3. Play the same video again
4. Right-click on video → "Stats for nerds"
5. Note the "Connection Speed" value

#### **What to include in your report:**

**Q5(a):** Throughput plot
- Graph: `q5_outputs/throughput_plot.png`
- Table showing throughput per second

**Q5(b):** Comparison with Stats for Nerds
- Your measured throughput vs YouTube's reported speed
- Explain differences: application-level vs network-level, overhead, adaptive bitrate

**Q5(c):** TCP Connection State
- Screenshot of: `netstat -an | grep ESTABLISHED | grep -E '(youtube|googlevideo)'`
- Or use Wireshark to view TCP streams and their states

---

## 📊 Creating Your Report

### Report Structure:

```
1. Introduction
2. Question 1: Traceroute and Ping Analysis
   - Screenshots and outputs
   - Analysis for parts (a) through (g)
3. Question 2: DNS Analysis
   - DNS server information
   - Hop count analysis
4. Question 3: Localhost Ping Failure
   - Screenshot
   - Explanation
5. Question 4: PCAP Analysis
   - All parts (a) through (g)
   - Include the generated plot
6. Question 5: YouTube Streaming
   - Throughput analysis
   - Stats for Nerds comparison
   - TCP connection states
7. Conclusion
```

### Include These Files in Your Submission:

```
<Roll_No>-assignment1/
├── report.pdf (or .docx)
├── q1_traceroute_ping.sh
├── q1_analysis.py
├── q2_dns_analysis.sh
├── q3_localhost_fail.sh
├── q4_pcap_parser.py
├── q4_plot_downloads.py
├── q5_youtube_analysis.py
├── q1_outputs/
├── q2_outputs/
├── q3_outputs/
├── q4_outputs/
└── q5_outputs/
```

---

## 🔧 Troubleshooting

### "Permission Denied" errors:
```bash
chmod +x *.sh *.py
```

### tcptraceroute/tcpping not found:
```bash
brew install tcptraceroute tcpping
```

### Python packages missing:
```bash
pip3 install matplotlib --break-system-packages
```

### macOS firewall blocking:
Go to System Preferences > Security & Privacy > Firewall > Allow tcptraceroute, tcpping

### Can't capture packets (tcpdump):
You need sudo: `sudo tcpdump ...`

---

## ⏰ Time Estimate

- Q1: 15-20 minutes
- Q2: 10-15 minutes  
- Q3: 5-10 minutes
- Q4: Already done! (5 minutes to review)
- Q5: 15-20 minutes
- Report writing: 30-45 minutes

**Total: ~1.5-2 hours**

---

## 📞 Notes

- All scripts create their own output directories
- Screenshots can be taken using macOS Screenshot tool (Cmd+Shift+4)
- For Q1, the analysis script estimates distances - you may need to adjust based on your actual location
- Q4 is already analyzed - just include the outputs in your report
- Make sure to explain your observations, not just present data

---

## ✅ Final Checklist

- [ ] Q1: Ran traceroute and ping scripts
- [ ] Q1: Analyzed results with q1_analysis.py
- [ ] Q1: Took screenshots of outputs
- [ ] Q2: Found DNS servers
- [ ] Q2: Compared hop counts
- [ ] Q3: Made ping fail to localhost
- [ ] Q3: Took screenshot of failure
- [ ] Q4: Reviewed PCAP analysis outputs
- [ ] Q4: Included download time plot
- [ ] Q5: Captured YouTube traffic
- [ ] Q5: Analyzed throughput
- [ ] Q5: Compared with Stats for Nerds
- [ ] Q5: Checked TCP connection states
- [ ] Created comprehensive report with all screenshots
- [ ] Packaged all code files
- [ ] Named folder correctly: <Roll_No>-assignment1
- [ ] Created .tar.gz archive

Good luck with your assignment! 🎓
