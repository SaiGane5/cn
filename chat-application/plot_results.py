import os
import pandas as pd
import matplotlib.pyplot as plt

# --- Latency distribution plot ---
fig, axes = plt.subplots(1, 3, figsize=(15, 5), sharey=True)
fig.suptitle("Message Delivery Time Distribution")

servers = [
    ("latency_logs_thread",  "Thread-based",  axes[0]),
    ("latency_logs_fork",    "Fork-based",    axes[1]),
    ("latency_logs_select",  "Select-based",  axes[2]),
]

for logdir, label, ax in servers:
    all_lat = []
    if os.path.exists(logdir):
        for f in os.listdir(logdir):
            df = pd.read_csv(os.path.join(logdir, f))
            all_lat.extend(df["latency_ms"].tolist())
    if all_lat:
        ax.hist(all_lat, bins=30, edgecolor='black')
    ax.set_title(label)
    ax.set_xlabel("Latency (ms)")
    ax.set_ylabel("Frequency")

plt.tight_layout()
plt.savefig("latency_distribution.png", dpi=150)
plt.close()
print("Saved latency_distribution.png")

# --- CPU and Memory vs clients ---
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
fig.suptitle("CPU & Memory Usage vs Number of Clients")

metric_files = [
    ("metrics_thread.log", "Thread"),
    ("metrics_fork.log",   "Fork"),
    ("metrics_select.log", "Select"),
]

for fname, label in metric_files:
    if not os.path.exists(fname): continue
    df = pd.read_csv(fname)
    # Use row index as proxy for time/load
    df["sample"] = range(len(df))
    ax1.plot(df["sample"], df["cpu_pct"],    label=label, marker='o', markersize=3)
    ax2.plot(df["sample"], df["vmrss_kb"]/1024, label=label, marker='s', markersize=3)

ax1.set_title("CPU Usage")
ax1.set_xlabel("Sample (every 5s)")
ax1.set_ylabel("CPU %")
ax1.legend()

ax2.set_title("Memory Usage (VmRSS)")
ax2.set_xlabel("Sample (every 5s)")
ax2.set_ylabel("Memory (MB)")
ax2.legend()

plt.tight_layout()
plt.savefig("cpu_memory_comparison.png", dpi=150)
plt.close()
print("Saved cpu_memory_comparison.png")