# 🚀 QUICK START GUIDE
## CS 3205 Assignment 1 - Complete Before EOD

## ⏰ TIMELINE (Total: ~2 hours)

### Phase 1: Setup (10 mins)
```bash
cd ~/Downloads  # or wherever you saved the package
# Extract the files
chmod +x *.sh
```

### Phase 2: Installation (5 mins)
```bash
./install_tools.sh
```
**Note:** Press Enter when prompted, allow sudo access

### Phase 3: Run All Questions (30-45 mins)

#### OPTION A: Automated (Recommended)
```bash
./run_all.sh
```
Follow the prompts and take screenshots as you go.

#### OPTION B: Manual (Step by step)

**Q1 - Traceroute & Ping (15 mins):**
```bash
./q1_traceroute_ping.sh
python3 q1_analysis.py | tee q1_results.txt
```
📸 Screenshot: Terminal output

**Q2 - DNS Analysis (5 mins):**
```bash
./q2_dns_analysis.sh
```
📸 Screenshot: DNS server info

**Q3 - Localhost Fail (5 mins):**
```bash
./q3_localhost_fail.sh
sudo pfctl -e -f q3_outputs/pf_block.conf
ping -c 5 127.0.0.1
```
📸 Screenshot: Failed ping (100% packet loss)
```bash
sudo pfctl -d  # IMPORTANT: Restore firewall!
```

**Q4 - PCAP Analysis (DONE!):**
```bash
python3 q4_pcap_parser.py
python3 q4_plot_downloads.py
```
✅ All results already in q4_outputs/

**Q5 - YouTube Capture (15 mins):**

Terminal 1:
```bash
# Find your interface first:
route get google.com | grep interface
# Example output: interface: en0

# Start capture (replace en0 with your interface):
sudo tcpdump -i en0 -w q5_outputs/youtube_capture.pcap 'host googlevideo.com or host youtube.com'
```

Terminal 2/Browser:
1. Open YouTube
2. Play ANY video
3. Wait EXACTLY 30 seconds
4. Stop video

Terminal 1:
Press Ctrl+C to stop capture

Then analyze:
```bash
python3 q5_youtube_analysis.py
```

Enable YouTube Stats for Nerds:
- Right-click video → "Stats for nerds"
- Note the "Connection Speed" value
📸 Screenshot: Stats for nerds + terminal output

TCP connection state:
```bash
netstat -an | grep ESTABLISHED | grep -E '(youtube|googlevideo)'
```
📸 Screenshot: Active connections

---

### Phase 4: Create Report (30-45 mins)

Open `report_template.md` and fill in:

**Must Include:**
1. ✅ All screenshots (Q1, Q2, Q3, Q5)
2. ✅ Q4 plot: `q4_outputs/download_times_plot.png`
3. ✅ Explanations for each question
4. ✅ Analysis of results

**Export to PDF:**
- Use Markdown to PDF converter
- OR copy to Word/Google Docs and export

---

### Phase 5: Package Submission (5 mins)

```bash
# Create your submission folder
mkdir YourRollNumber-assignment1

# Copy all required files
cp -r *.sh *.py q*_outputs/ YourRollNumber-assignment1/
cp report.pdf YourRollNumber-assignment1/

# Create tar.gz
tar -czf YourRollNumber-assignment1.tar.gz YourRollNumber-assignment1/
```

---

## 📋 SUBMISSION CHECKLIST

- [ ] Q1: Traceroute screenshots ✓
- [ ] Q1: Ping screenshots ✓
- [ ] Q1: Analysis complete ✓
- [ ] Q2: DNS server info ✓
- [ ] Q2: Hop comparison ✓
- [ ] Q3: Failed ping screenshot ✓
- [ ] Q3: Explanation written ✓
- [ ] Q4: All parts analyzed (already done!) ✓
- [ ] Q4: Download plot included ✓
- [ ] Q5: YouTube capture complete ✓
- [ ] Q5: Throughput plot ✓
- [ ] Q5: Stats for Nerds comparison ✓
- [ ] Q5: TCP states screenshot ✓
- [ ] Report: All sections filled ✓
- [ ] Report: Exported to PDF ✓
- [ ] Code: All .sh and .py files ✓
- [ ] Archive: .tar.gz created ✓

---

## ⚠️ COMMON ISSUES

**"Permission denied" on .sh files:**
```bash
chmod +x *.sh
```

**"tcptraceroute not found":**
```bash
brew install tcptraceroute tcpping
```

**Can't capture packets:**
- Need sudo: `sudo tcpdump ...`
- Check interface name: `ifconfig`

**Q5 capture empty:**
- Make sure video is actually playing
- Check you're using correct interface (en0/en1)
- YouTube might be using different domains - try: `host youtube.com or host googlevideo.com or host ytimg.com`

**macOS firewall blocking:**
- System Preferences → Security & Privacy → Firewall
- Allow tcptraceroute, tcpping

---

## 🎯 PRIORITY IF SHORT ON TIME

If you're really pressed for time, prioritize:

1. **HIGH PRIORITY:**
   - Q4 (already done - just review outputs!)
   - Q1 (run scripts, take screenshots)
   - Q5 (capture and analyze)

2. **MEDIUM PRIORITY:**
   - Q2 (relatively quick)
   - Q3 (quick to demonstrate)

3. **REPORT:**
   - Use the template
   - Include all screenshots
   - Brief but clear explanations

---

## 💡 PRO TIPS

1. **Take screenshots as you go** - Don't wait until the end
2. **Run commands in screen/tmux** - So you can reference output later
3. **Save all terminal outputs** - Use `| tee filename.txt`
4. **Q4 is done!** - Focus on other questions
5. **Test before capturing** - Make sure tools work before Q5
6. **Read error messages** - They usually tell you what's wrong

---

## 📞 NEED HELP?

Check these first:
1. README.md - Detailed instructions
2. report_template.md - Shows what to include
3. Error messages - Usually self-explanatory

---

**YOU CAN DO THIS! 💪**

The scripts are ready, Q4 is done, just need to run them and document results!

Estimated time with this guide: **1.5-2 hours total**

Good luck! 🎓
