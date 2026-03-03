"""
plot_results.py – generate the two performance plots required by Assignment 2.

Plot 1 – latency_distribution.png
    Histogram of one-way message delivery time for each server type
    (fork, thread, select).  Data source: latency_logs_{type}/client_*.csv

Plot 2 – cpu_memory_comparison.png
    CPU usage (%) and RSS memory (MB) vs number of concurrent clients.
    Data source (preferred): stress_summary_{type}.log
        columns: num_clients,vmrss_kb,pss_kb,cpu_pct
        one row per load level (1,2,4,6,8,10 clients).
    Fallback: metrics_{type}.log (raw time-series, sample index as x-axis).
"""

import os
import pandas as pd
import matplotlib
matplotlib.use("Agg")          # non-interactive backend
import matplotlib.pyplot as plt

SERVER_TYPES = [
    ("thread", "Thread-based", "#1f77b4"),
    ("fork",   "Fork-based",   "#ff7f0e"),
    ("select", "Select-based", "#2ca02c"),
]

# ------------------------------------------------------------------ #
# Plot 1 – Message delivery time distribution (latency histograms)
# ------------------------------------------------------------------ #
fig, axes = plt.subplots(1, 3, figsize=(15, 5), sharey=False)
fig.suptitle("Message Delivery Time Distribution\n(one-way latency, broadcast via loopback)", fontsize=13)

for ax, (stype, label, color) in zip(axes, SERVER_TYPES):
    logdir = f"latency_logs_{stype}"
    all_lat = []
    if os.path.isdir(logdir):
        for fname in sorted(os.listdir(logdir)):
            if not fname.endswith(".csv"):
                continue
            fpath = os.path.join(logdir, fname)
            try:
                df = pd.read_csv(fpath)
                if "latency_ms" in df.columns:
                    vals = df["latency_ms"].dropna().tolist()
                    all_lat.extend([v for v in vals if 0 <= v < 5000])
            except Exception:
                pass

    if all_lat:
        ax.hist(all_lat, bins=30, color=color, edgecolor="black", alpha=0.8)
        med = sorted(all_lat)[len(all_lat) // 2]
        ax.axvline(med, color="red", linestyle="--", linewidth=1.5,
                   label=f"median = {med:.1f} ms")
        ax.legend(fontsize=9)
    else:
        ax.text(0.5, 0.5, "No data", transform=ax.transAxes,
                ha="center", va="center", fontsize=12, color="gray")

    ax.set_title(label, fontsize=11)
    ax.set_xlabel("Latency (ms)")
    ax.set_ylabel("Frequency")
    n = len(all_lat)
    ax.set_title(f"{label}\n(n={n} messages)", fontsize=11)

plt.tight_layout()
plt.savefig("latency_distribution.png", dpi=150)
plt.close()
print("Saved latency_distribution.png")

# ------------------------------------------------------------------ #
# Plot 2 – CPU and memory vs number of clients
# ------------------------------------------------------------------ #
fig, (ax_cpu, ax_mem) = plt.subplots(1, 2, figsize=(13, 5))
fig.suptitle("CPU & Memory Usage vs Number of Concurrent Clients", fontsize=13)

any_data = False

for stype, label, color in SERVER_TYPES:
    summary_file = f"stress_summary_{stype}.log"
    metrics_file = f"metrics_{stype}.log"

    df = None
    x_col  = None
    x_label = "Number of clients"

    # --- preferred: stress_summary has explicit client count column ---
    if os.path.isfile(summary_file):
        try:
            tmp = pd.read_csv(summary_file)
            if {"num_clients", "vmrss_kb", "cpu_pct"}.issubset(tmp.columns):
                # Drop rows where all metrics are zero (server was not running)
                tmp = tmp[~((tmp["vmrss_kb"] == 0) & (tmp["cpu_pct"] == 0))]
                if len(tmp) >= 2:
                    df      = tmp
                    x_col   = "num_clients"
                    x_label = "Number of clients"
        except Exception:
            pass

    # --- fallback: raw metrics time series ---
    if df is None and os.path.isfile(metrics_file):
        try:
            tmp = pd.read_csv(metrics_file)
            if {"vmrss_kb", "cpu_pct"}.issubset(tmp.columns):
                tmp = tmp[~((tmp["vmrss_kb"] == 0) & (tmp["cpu_pct"] == 0))]
                if len(tmp) >= 2:
                    tmp["sample"] = range(len(tmp))
                    df      = tmp
                    x_col   = "sample"
                    x_label = "Sample index (every 5 s)"
        except Exception:
            pass

    if df is None or x_col is None:
        continue

    any_data = True
    xs = df[x_col]

    ax_cpu.plot(xs, df["cpu_pct"],
                label=label, color=color, marker="o", markersize=4, linewidth=1.8)
    ax_mem.plot(xs, df["vmrss_kb"] / 1024,
                label=label, color=color, marker="s", markersize=4, linewidth=1.8)

if not any_data:
    for ax in (ax_cpu, ax_mem):
        ax.text(0.5, 0.5,
                "No data yet.\nRun test_stress.sh first.",
                transform=ax.transAxes, ha="center", va="center",
                fontsize=11, color="gray")

ax_cpu.set_title("CPU Usage")
ax_cpu.set_xlabel(x_label if any_data else "")
ax_cpu.set_ylabel("CPU (%)")
if any_data: ax_cpu.legend()
ax_cpu.grid(True, linestyle="--", alpha=0.4)

ax_mem.set_title("Memory Usage (VmRSS)")
ax_mem.set_xlabel(x_label if any_data else "")
ax_mem.set_ylabel("Memory (MB)")
if any_data: ax_mem.legend()
ax_mem.grid(True, linestyle="--", alpha=0.4)

plt.tight_layout()
plt.savefig("cpu_memory_comparison.png", dpi=150)
plt.close()
print("Saved cpu_memory_comparison.png")
