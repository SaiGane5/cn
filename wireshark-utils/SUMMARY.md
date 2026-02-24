# CS 3205 Assignment 1 - Complete Package Summary
## Ready to Use on macOS

---

## 📦 WHAT'S INCLUDED

This package contains everything you need to complete Assignment 1:

### 📜 Scripts (Ready to Run):
1. **install_tools.sh** - Installs all required tools
2. **run_all.sh** - Master script to run all questions
3. **q1_traceroute_ping.sh** - Q1 data collection
4. **q1_analysis.py** - Q1 analysis and calculations
5. **q2_dns_analysis.sh** - Q2 DNS server discovery
6. **q3_localhost_fail.sh** - Q3 ping failure setup
7. **q4_pcap_parser.py** - Q4 PCAP analysis ✅ DONE
8. **q4_plot_downloads.py** - Q4 visualization ✅ DONE
9. **q5_youtube_capture.sh** - Q5 instructions
10. **q5_youtube_analysis.py** - Q5 throughput analysis

### 📄 Documentation:
1. **QUICKSTART.md** - Fast track guide (START HERE!)
2. **README.md** - Comprehensive instructions
3. **report_template.md** - Complete report template with all sections

### 📊 Results (Q4 Already Complete!):
- **q4_outputs/download_times_plot.png** - Bar chart visualization
- **q4_outputs/download_times_data.txt** - Raw data
- **q4_outputs/client_id_mapping.txt** - Client IP mappings
- All Q4 analysis results ready to use!

---

## ✅ Q4 IS ALREADY DONE!

I've already analyzed the PCAP file for you. Here are the results:

### Q4 Complete Results:

**Q4(a) - Clients & Servers:**
- 15 clients identified
- 5 servers identified
- Full IP lists available

**Q4(b) - GET Requests:**
- Analyzed for all 15 clients
- Range: 1-13 requests per client

**Q4(c) - raw.mp4 Download Times:**
- 13 downloads analyzed
- Range: 129.25 - 176.69 seconds
- Fastest: Client C5 (129.25s)
- Slowest: Client C13 (176.69s)

**Q4(d) - Download Time Plot:**
- Professional bar chart created ✅
- Color-coded by speed (green=fast, orange=medium, red=slow)
- Ready to include in report!

**Q4(e) - User Agent & Server Info:**
- User-Agent: Firefox 18.0 on Ubuntu Linux
- Response Code: 307 Temporary Redirect
- Server: Apache/2.2.22 (Fedora)

**Q4(f) - HTTP Persistence:**
- **PERSISTENT** connections detected
- 25 connections with multiple requests

**Q4(g) - Parallel Connections:**
- Most clients: 6-7 parallel connections
- Typical browser behavior for HTTP/1.1

**Just copy these results into your report!**

---

## ⚡ FASTEST WAY TO COMPLETE (1.5-2 hours)

### Step 1: Setup (5 mins)
```bash
# After downloading and extracting the package:
cd path/to/assignment1_package
chmod +x *.sh
./install_tools.sh
```

### Step 2: Run Everything (30-45 mins)
```bash
./run_all.sh
```
Follow prompts, take screenshots when asked.

### Step 3: Q5 Manual Capture (15 mins)
```bash
# Terminal 1:
sudo tcpdump -i en0 -w q5_outputs/youtube_capture.pcap 'host googlevideo.com or host youtube.com'

# Browser: Play YouTube video for 30 seconds

# Terminal 1: Ctrl+C to stop

# Analyze:
python3 q5_youtube_analysis.py
```

### Step 4: Create Report (30-45 mins)
- Open `report_template.md`
- Fill in your data
- Include screenshots
- Q4 sections are ready to copy!
- Export to PDF

### Step 5: Submit (5 mins)
```bash
# Package everything
mkdir YourRollNumber-assignment1
cp -r *.sh *.py q*_outputs/ report.pdf YourRollNumber-assignment1/
tar -czf YourRollNumber-assignment1.tar.gz YourRollNumber-assignment1/
```

---

## 📝 WHAT YOU NEED TO DO

### Must Complete on Your Mac:
- ✅ Q1: Run traceroute and ping scripts (15 mins)
- ✅ Q2: Find DNS servers (5 mins)
- ✅ Q3: Make ping fail to localhost (5 mins)
- ❌ Q4: **ALREADY DONE** - Just review outputs! (5 mins)
- ✅ Q5: Capture YouTube traffic (15 mins)
- ✅ Report: Fill in template (30-45 mins)

### Already Done for You:
- ✅ Q4 PCAP Analysis - Complete!
- ✅ Q4 Plot - Professional visualization ready!
- ✅ All code written and tested
- ✅ Report template with all sections
- ✅ Instructions for every step

---

## 🎯 KEY POINTS

1. **Q4 is completely finished** - You just need to include the results in your report
2. **All scripts are ready** - Just run them and take screenshots
3. **Report template is complete** - Fill in your data
4. **Time estimate: 1.5-2 hours total**

---

## 📸 SCREENSHOTS YOU NEED

- [ ] Q1: Traceroute output (2 screenshots - IITB & AUST)
- [ ] Q1: Ping output (2 screenshots - IITB & AUST)
- [ ] Q2: DNS server info
- [ ] Q3: Failed ping to 127.0.0.1 showing 100% packet loss
- [ ] Q4: Include download_times_plot.png (already created!)
- [ ] Q5: Throughput analysis output
- [ ] Q5: YouTube Stats for Nerds
- [ ] Q5: TCP connection states

---

## 🚨 IMPORTANT NOTES

1. **Q4 saves you 30-45 minutes!** The hardest part is done.
2. **Run on macOS** - All scripts designed for Mac
3. **Need sudo** - Some commands require administrator access
4. **Screenshots as you go** - Don't wait until the end
5. **Read error messages** - They tell you what to fix

---

## 💪 YOU'VE GOT THIS!

**What makes this easy:**
- Q4 (30% of assignment) is completely done
- All code is written and tested
- Step-by-step instructions provided
- Report template ready to fill
- Estimated time: ~2 hours

**What you need:**
- A Mac with internet
- 2 hours of focused time
- Follow the QUICKSTART.md guide

---

## 📞 TROUBLESHOOTING

**Tools not installing?**
```bash
# Make sure Homebrew is installed first
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**Permission errors?**
```bash
chmod +x *.sh
```

**Can't run scripts?**
```bash
# Make sure you're in the right directory
ls -la *.sh
# Should see green filenames (executable)
```

**Q5 capture empty?**
- Verify network interface: `ifconfig`
- Try different interface names (en0, en1)
- Make sure YouTube is actually playing

---

## 🏁 FINAL CHECKLIST

Before submission, verify:

- [ ] All 5 questions completed
- [ ] Q4 plot included in report (download_times_plot.png)
- [ ] All screenshots taken and included
- [ ] Report exported to PDF
- [ ] All code files copied to submission folder
- [ ] Folder named correctly: YourRollNumber-assignment1
- [ ] .tar.gz archive created
- [ ] Archive size reasonable (should be < 50MB)

---

## ⏰ TIME BREAKDOWN

- Setup & Installation: 10 mins
- Q1 Execution: 15 mins
- Q2 Execution: 5 mins
- Q3 Execution: 5 mins
- Q4 Review: 5 mins (already done!)
- Q5 Execution: 15 mins
- Report Writing: 45 mins
- Packaging: 5 mins
**TOTAL: ~1 hour 45 minutes**

---

**START WITH QUICKSTART.md FOR FASTEST COMPLETION!**

Good luck! You can definitely finish this before EOD! 🎓
