# CS 3205: Computer Networks - Assignment 1
## Network Analysis and Protocol Investigation

**Student Name:** [Your Name]  
**Roll Number:** [Your Roll Number]  
**Date:** February 07, 2026

---

## Question 1: Traceroute and Ping Analysis

### (a) Traceroute Results

#### IIT Bombay (iitb.ac.in)

**Number of intermediate hops:** [Fill in]

**IP Addresses and Average Latencies:**

| Hop # | IP Address | Average Latency (ms) |
|-------|------------|---------------------|
| 1     | x.x.x.x    | x.xxx               |
| 2     | x.x.x.x    | x.xxx               |
| ...   | ...        | ...                 |

**Screenshot:**
[Insert traceroute_iitb.txt screenshot here]

#### African University of Science and Technology (aust.edu.ng)

**Number of intermediate hops:** [Fill in]

**IP Addresses and Average Latencies:**

| Hop # | IP Address | Average Latency (ms) |
|-------|------------|---------------------|
| 1     | x.x.x.x    | x.xxx               |
| 2     | x.x.x.x    | x.xxx               |
| ...   | ...        | ...                 |

**Screenshot:**
[Insert traceroute_aust.txt screenshot here]

---

### (b) Comparison of Hop Counts

**IITB Hops:** [Number]  
**AUST Hops:** [Number]  
**Difference:** [Number]

**Explanation of Latency Difference:**

[Your explanation should cover:]
- Geographic distance (IITB is in India, AUST is in Nigeria)
- Number of router hops (more hops = more processing and queuing delays)
- International vs domestic routing
- Network infrastructure quality
- Possible submarine cable routes for international traffic
- Peering agreements and routing policies

---

### (c) Ping to IITB

**Number of packets sent:** 100  
**Average latency:** [X.XXX] ms

**Screenshot:**
[Insert ping_iitb.txt screenshot here]

---

### (d) Ping to AUST

**Number of packets sent:** 100  
**Average latency:** [X.XXX] ms

**Screenshot:**
[Insert ping_aust.txt screenshot here]

---

### (e) Expected Latency Calculation

**Given:** Speed of light in fiber = 2×10⁸ m/s (approximately 2/3 of c)

#### IITB Calculation:

**Approximate distance from [Your City] to Mumbai:** ~[XXXX] km

**Expected RTT:**
- Distance (one way) = [XXXX] km = [XXXXXXXX] m
- Time (one way) = Distance / Speed = [XXXXXXXX] / (2×10⁸) = [X.XXXX] s
- RTT = 2 × Time (one way) = [X.XXX] ms

**Actual ping latency:** [X.XXX] ms  
**Difference:** [X.XXX] ms

#### AUST Calculation:

**Approximate distance from [Your City] to Abuja:** ~[XXXX] km

**Expected RTT:**
- Distance (one way) = [XXXX] km = [XXXXXXXX] m
- Time (one way) = Distance / Speed = [XXXXXXXX] / (2×10⁸) = [X.XXXX] s
- RTT = 2 × Time (one way) = [XX.XXX] ms

**Actual ping latency:** [XX.XXX] ms  
**Difference:** [XX.XXX] ms

**Analysis:**

The actual latency is significantly higher than the theoretical minimum because:

1. **Propagation delay** (speed of light limitation) - accounted for in calculation
2. **Routing overhead** - packets don't travel in a straight line
3. **Processing delays** - routers need time to process packets
4. **Queuing delays** - packets wait in router queues
5. **Transmission delays** - time to push bits onto the wire
6. **Multiple hops** - each hop adds delay
7. **Network congestion** - variable based on traffic load

---

### (f) Sum of Intermediate Hop Latencies

#### IITB:

**Sum of all hop latencies:** [XXX.XXX] ms  
**Direct ping latency:** [XX.XXX] ms  
**Difference:** [XXX.XXX] ms

#### AUST:

**Sum of all hop latencies:** [XXX.XXX] ms  
**Direct ping latency:** [XX.XXX] ms  
**Difference:** [XXX.XXX] ms

**Explanation:**

The sum of hop latencies does NOT match the direct ping latency because:

1. **Temporal variation** - Traceroute measures each hop at different times, while ping measures the entire path at once
2. **Network conditions change** - Traffic patterns, congestion, and routing may vary between measurements
3. **Different packet paths** - Some routers may use different routes for successive hops
4. **ICMP rate limiting** - Some routers rate-limit ICMP responses, causing artificially high latencies
5. **Traceroute overhead** - Additional processing for TTL expiration messages

The sum is typically [higher/lower] because [your observation].

---

### (g) Maximum Hop Latency

#### IITB:

**Maximum hop latency:** [XX.XXX] ms (at hop [N])  
**Direct ping latency:** [XX.XXX] ms

#### AUST:

**Maximum hop latency:** [XXX.XXX] ms (at hop [N])  
**Direct ping latency:** [XXX.XXX] ms

**Explanation:**

The maximum hop latency is [higher/lower] than the direct ping latency because:

[Your explanation should cover:]
- If max hop > ping: Could be due to ICMP rate limiting at that specific router, or that router being heavily loaded during traceroute but not during ping
- If max hop < ping: The direct ping includes cumulative delays from all hops, not just the maximum single hop
- Timing differences between when traceroute and ping were run
- The maximum hop may not represent the bottleneck for the entire path

---

## Question 2: DNS Server Analysis

### (a) DNS Server IP Addresses

#### Your Local DNS Server (Resolver):

**IP Address:** [x.x.x.x]

#### IITB Authoritative DNS Servers:

**DNS Server Names and IPs:**
- [dns1.iitb.ac.in] - [x.x.x.x]
- [dns2.iitb.ac.in] - [x.x.x.x]

#### AUST Authoritative DNS Servers:

**DNS Server Names and IPs:**
- [dns1.aust.edu.ng] - [x.x.x.x]
- [dns2.aust.edu.ng] - [x.x.x.x]

**Comparison with Q1:**

The DNS server IPs are [same as / different from] the web server IPs found in Q1 because:
[Explanation: DNS servers are typically separate from web servers for security, load distribution, and organizational reasons]

---

### (b) Traceroute to DNS Servers vs Web Servers

#### IITB:

**Hops to web server (from Q1):** [N] hops  
**Hops to DNS server:** [N] hops  
**Difference:** [N] hops

#### AUST:

**Hops to web server (from Q1):** [N] hops  
**Hops to DNS server:** [N] hops  
**Difference:** [N] hops

**Reason for difference:**

[Explanation should cover:]
- Different physical locations of DNS and web servers
- DNS servers may be distributed geographically for redundancy
- Content Delivery Networks (CDNs) for web servers
- DNS hierarchy and caching

**Is this DNS server a Local Name Server or Root Name Server?**

Based on the number of hops and response time, this is likely a [Local Name Server / Authoritative Name Server] because:

- **If few hops (~1-3):** Likely a local DNS resolver/forwarder from your ISP
- **If many hops (~5-15):** Likely an authoritative name server at the organization
- **Root Name Servers:** Would show ~4-8 hops and be at well-known IP addresses (e.g., 198.41.0.4 for a.root-servers.net)

[Your analysis here]

---

## Question 3: Making Ping Fail to 127.0.0.1

### Method Used

**Method:** Packet Filter (pfctl) firewall rule

### Steps Taken

1. Created firewall configuration file (`pf_block.conf`) with rule:
   ```
   block drop quick on lo0 from any to 127.0.0.1
   block drop quick on lo0 from 127.0.0.1 to any
   ```

2. Applied the rule:
   ```bash
   sudo pfctl -e -f q3_outputs/pf_block.conf
   ```

3. Tested ping:
   ```bash
   ping -c 5 127.0.0.1
   ```

4. Restored normal operation:
   ```bash
   sudo pfctl -d
   ```

### Screenshot

[Insert screenshot showing ping failure with 100% packet loss]

### Explanation

**How it works:**

The macOS packet filter (pf) firewall intercepts all packets on the loopback interface (lo0) and drops any packets destined to or from 127.0.0.1 before they reach the network stack. This causes:

1. Ping packets to be dropped immediately
2. No ICMP Echo Reply messages generated
3. 100% packet loss
4. Destination unreachable or timeout errors

**Why it fails:**

- The firewall rule prevents packets from reaching the loopback device
- No response can be generated because requests are blocked
- The operating system's networking layer never sees the ping request

**Alternative method:** Route blackhole - redirects traffic to a non-existent host

---

## Question 4: PCAP Analysis - Online Quiz Classroom

### (a) Clients and Servers

#### Server IP Addresses:
- 10.129.23.153
- 10.129.5.192
- 10.129.50.6
- 108.160.162.112
- 108.160.162.51

#### Client IP Addresses:
- 10.129.26.103
- 10.129.26.154
- 10.129.26.155
- 10.129.26.62
- 10.129.26.72
- 10.129.26.74
- 10.129.26.87
- 10.129.28.105
- 10.129.28.106
- 10.129.28.180
- 10.129.28.207
- 10.129.28.223
- 10.129.28.29
- 10.129.5.192
- 10.129.50.5

**Total:** 15 clients, 5 servers

**Screenshot:**
[Insert screenshot of client/server identification output]

---

### (b) GET Requests per Client

| Client IP        | Number of GET Requests |
|-----------------|------------------------|
| 10.129.26.103   | 10                     |
| 10.129.26.154   | 11                     |
| 10.129.26.155   | 11                     |
| 10.129.26.62    | 1                      |
| 10.129.26.72    | 11                     |
| 10.129.26.74    | 11                     |
| 10.129.26.87    | 10                     |
| 10.129.28.105   | 11                     |
| 10.129.28.106   | 11                     |
| 10.129.28.180   | 10                     |
| 10.129.28.207   | 11                     |
| 10.129.28.223   | 13                     |
| 10.129.28.29    | 10                     |
| 10.129.5.192    | 5                      |
| 10.129.50.5     | 1                      |

**Observation:** Most clients made 10-11 GET requests, suggesting they downloaded similar content (quiz page + resources). Some clients (e.g., 10.129.28.223 with 13 requests) may have refreshed or requested additional resources.

---

### (c) Download Time of raw.mp4

| Client IP        | Download Time (seconds) |
|-----------------|-------------------------|
| 10.129.26.74    | 129.25                  |
| 10.129.26.72    | 144.11                  |
| 10.129.26.87    | 148.36                  |
| 10.129.28.180   | 149.54                  |
| 10.129.28.106   | 154.83                  |
| 10.129.26.155   | 157.13                  |
| 10.129.26.103   | 160.83                  |
| 10.129.28.207   | 160.81                  |
| 10.129.28.223   | 164.22                  |
| 10.129.26.154   | 169.72                  |
| 10.129.28.223   | 172.70                  |
| 10.129.28.105   | 175.79                  |
| 10.129.28.29    | 176.69                  |

**Analysis:**

- **Fastest download:** 129.25 seconds (Client 10.129.26.74)
- **Slowest download:** 176.69 seconds (Client 10.129.28.29)
- **Range:** 47.44 seconds difference

**Possible reasons for variation:**
1. Network congestion - multiple students downloading simultaneously
2. Different network paths or switch ports
3. Client-side processing speed
4. Server load balancing
5. TCP congestion control variations

---

### (d) Download Time Graph

[Insert q4_outputs/download_times_plot.png here]

**Graph shows:** 
- Download times vary between ~130-177 seconds
- Client 10.129.26.74 had the fastest download
- Client 10.129.28.29 had the slowest download
- Most downloads clustered around 150-170 seconds

---

### (e) User Agent, Response Code, and Server Info

**Client analyzed:** 10.129.26.62

**User-Agent:**
```
Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:18.0) Gecko/20100101 Firefox/18.0
```

**HTTP Response Code:** 307

**Response Description:** Temporary Redirect

**Web Server:** Apache/2.2.22 (Fedora)

**Analysis:**

- **Browser:** Firefox 18.0 on Ubuntu Linux (32-bit)
- **Response 307:** Temporary redirect indicates the server is redirecting the request to another location while preserving the HTTP method
- **Server:** Apache 2.2.22 running on Fedora Linux
- This suggests the classroom server is running a Linux-based system with Apache web server

---

### (f) HTTP Persistence

**Analysis Result:** HTTP is **PERSISTENT**

**Evidence:**
- Found 25 TCP connections that handled multiple HTTP requests
- This indicates connection reuse (HTTP/1.1 persistent connections)

**Explanation:**

HTTP Persistent Connections (HTTP/1.1) allow multiple HTTP requests/responses over a single TCP connection:

**Advantages:**
1. Reduced latency (no need for multiple TCP handshakes)
2. Lower CPU usage (fewer connections to manage)
3. Reduced network congestion (fewer TCP connections)
4. Better throughput for multiple small objects

**How to identify:**
- `Connection: keep-alive` header in HTTP requests/responses
- Multiple HTTP requests using the same TCP source port
- Same TCP stream ID for multiple HTTP transactions

---

### (g) Number of Parallel Connections

| Client IP        | Parallel Connections |
|-----------------|---------------------|
| 10.129.26.103   | 8                   |
| 10.129.26.154   | 7                   |
| 10.129.26.155   | 7                   |
| 10.129.26.62    | 1                   |
| 10.129.26.72    | 7                   |
| 10.129.26.74    | 7                   |
| 10.129.26.87    | 7                   |
| 10.129.28.105   | 7                   |
| 10.129.28.106   | 6                   |
| 10.129.28.180   | 6                   |
| 10.129.28.207   | 8                   |
| 10.129.28.223   | 9                   |
| 10.129.28.29    | 6                   |
| 10.129.5.192    | 5                   |
| 10.129.50.5     | 1                   |

**Observation:**

- Most browsers opened 6-7 parallel connections
- This is typical for HTTP/1.1 browsers to speed up page loading
- Modern browsers limit parallel connections to prevent server overload
- Chrome/Firefox typically use 6-8 parallel connections per domain
- More connections = faster parallel downloads but more overhead

**Why parallel connections?**

1. **Faster page loading** - Download multiple resources simultaneously
2. **Overcome head-of-line blocking** - If one request is slow, others can proceed
3. **Maximize bandwidth utilization** - Multiple TCP streams can better utilize available bandwidth
4. **Browser optimization** - Balance between speed and resource usage

---

## Question 5: YouTube Streaming Analysis

### (a) Throughput Analysis

**Capture duration:** 30 seconds

**Throughput per second:**

| Time (s) | Throughput (Mbps) | Bytes Transferred |
|----------|-------------------|-------------------|
| 0        | [X.XXX]           | [XXXXX]           |
| 1        | [X.XXX]           | [XXXXX]           |
| ...      | ...               | ...               |
| 29       | [X.XXX]           | [XXXXX]           |

**Statistics:**
- **Average throughput:** [X.XXX] Mbps
- **Maximum throughput:** [X.XXX] Mbps
- **Minimum throughput:** [X.XXX] Mbps

**Graph:**
[Insert q5_outputs/throughput_plot.png here]

**Observations:**

[Describe patterns you see:]
- Initial buffering phase (high throughput)
- Steady streaming phase
- Any drops or spikes in throughput
- Adaptive bitrate changes

---

### (b) Comparison with YouTube Stats for Nerds

**Your measured average throughput:** [X.XXX] Mbps

**YouTube reported connection speed:** [X.XXX] Mbps

**Do they match?** [Yes/No - they are close / significantly different]

**Explanation:**

[Your explanation should cover:]

1. **If similar:**
   - Both measure network layer throughput
   - YouTube's measurement may average over a longer window
   - Confirms accuracy of our packet capture method

2. **If different (yours higher):**
   - Your measurement includes ALL IP/TCP overhead (headers, ACKs, retransmissions)
   - YouTube reports application-level throughput (actual video data)
   - Overhead can be 5-15% of total traffic
   - You may have captured other background traffic

3. **If different (yours lower):**
   - Your capture may have missed some packets
   - Filter might not have caught all YouTube domains
   - YouTube uses adaptive bitrate - may have increased quality during their measurement
   - Timing window differences

**Calculation of overhead:**

If your throughput > YouTube's:
- Overhead = (Your throughput - YouTube throughput) / Your throughput × 100%
- Example: (5.0 - 4.5) / 5.0 × 100% = 10% overhead

---

### (c) TCP Connection State

**Screenshot of active connections:**
[Insert screenshot of: netstat -an | grep ESTABLISHED | grep -E '(youtube|googlevideo)']

**Analysis:**

**Number of ESTABLISHED connections:** [N]

**Connection details:**

| Local Address:Port | Foreign Address:Port | State        |
|-------------------|---------------------|--------------|
| [x.x.x.x:xxxxx]   | [x.x.x.x:443]       | ESTABLISHED  |
| [x.x.x.x:xxxxx]   | [x.x.x.x:443]       | ESTABLISHED  |
| ...               | ...                 | ...          |

**Observations:**

1. **Multiple connections** - YouTube/Google uses multiple parallel connections for:
   - Video data
   - Audio data
   - Metadata and thumbnails
   - Analytics and tracking

2. **HTTPS (Port 443)** - All connections encrypted with TLS

3. **Connection states seen:**
   - ESTABLISHED - Active data transfer
   - TIME_WAIT - Connection closing (if any)
   - CLOSE_WAIT - Half-closed connection (if any)

4. **Google's server IPs** - Typically from these ranges:
   - 172.217.x.x (Google)
   - 142.250.x.x (Google)
   - 74.125.x.x (YouTube)

---

## Conclusion

This assignment provided hands-on experience with network analysis tools and protocols:

1. **Traceroute and Ping** - Demonstrated how geographic distance and routing affect latency
2. **DNS** - Showed the hierarchical nature of DNS and difference between resolvers and authoritative servers
3. **Firewall** - Illustrated how packet filtering can control network traffic
4. **HTTP Analysis** - Revealed persistent connections and parallel downloads in web traffic
5. **Streaming Analysis** - Demonstrated real-time bandwidth measurement and adaptive bitrate streaming

**Key Learnings:**

- Network latency is not just propagation delay - processing, queuing, and routing add significant overhead
- Modern web browsers use sophisticated techniques (persistent connections, parallel downloads) to optimize performance
- Protocol analysis tools like Wireshark/tcpdump are essential for understanding network behavior
- Real-world measurements often differ from theoretical calculations due to network complexity

---

## References

1. Kurose, J. F., & Ross, K. W. (2021). Computer Networking: A Top-Down Approach (8th ed.)
2. tcptraceroute documentation: https://github.com/mct/tcptraceroute
3. Wireshark User's Guide: https://www.wireshark.org/docs/
4. RFC 2616 - HTTP/1.1: https://tools.ietf.org/html/rfc2616
5. Course lecture slides and materials

---

**End of Report**
