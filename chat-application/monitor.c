#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/resource.h>
#include "monitor.h"

#ifdef __APPLE__
#include <mach/mach.h>
#endif

static const char *log_path;

static void read_proc_mem(long *vmrss_kb, long *pss_kb) {
    *vmrss_kb = 0; *pss_kb = 0;

#ifdef __APPLE__
    /* macOS: use Mach task_info for resident set size */
    struct mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  (task_info_t)&info, &count) == KERN_SUCCESS) {
        *vmrss_kb = (long)(info.resident_size / 1024);
        *pss_kb   = *vmrss_kb;  /* PSS not available on macOS; use RSS */
    }
#else
    /* Linux: read from /proc/self/status and /proc/self/smaps_rollup */
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0)
            sscanf(line + 6, "%ld", vmrss_kb);
    }
    fclose(f);

    f = fopen("/proc/self/smaps_rollup", "r");
    if (!f) return;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "Pss:", 4) == 0) {
            sscanf(line + 4, "%ld", pss_kb);
            break;
        }
    }
    fclose(f);
#endif
}

static void read_proc_cpu(double *cpu_pct) {
    *cpu_pct = 0.0;

#ifdef __APPLE__
    /* macOS: use getrusage to track CPU time delta */
    static long long prev_cpu_usec = 0;
    static long long prev_wall_ms  = 0;

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    long long cpu_usec = (long long)usage.ru_utime.tv_sec  * 1000000
                       + (long long)usage.ru_utime.tv_usec
                       + (long long)usage.ru_stime.tv_sec  * 1000000
                       + (long long)usage.ru_stime.tv_usec;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long wall_ms = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    if (prev_wall_ms > 0 && wall_ms > prev_wall_ms) {
        double delta_cpu_ms  = (double)(cpu_usec - prev_cpu_usec) / 1000.0;
        double delta_wall_ms = (double)(wall_ms  - prev_wall_ms);
        *cpu_pct = 100.0 * delta_cpu_ms / delta_wall_ms;
    }

    prev_cpu_usec = cpu_usec;
    prev_wall_ms  = wall_ms;
#else
    /* Linux: parse /proc/self/stat for jiffies */
    static long prev_utime = 0, prev_stime = 0;
    static long long prev_wall = 0;

    FILE *f = fopen("/proc/self/stat", "r");
    if (!f) return;
    long utime, stime;
    int dummy_i; char dummy_c;
    fscanf(f, "%d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld",
           &dummy_i, &dummy_c, &utime, &stime);
    fclose(f);

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long wall_ms = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    long hz = sysconf(_SC_CLK_TCK);
    long delta_cpu   = (utime + stime) - (prev_utime + prev_stime);
    long long delta_wall = wall_ms - prev_wall;

    if (prev_wall > 0 && delta_wall > 0)
        *cpu_pct = 100.0 * ((double)delta_cpu / hz) / ((double)delta_wall / 1000.0);

    prev_utime = utime; prev_stime = stime; prev_wall = wall_ms;
#endif
}

static void *monitor_loop(void *arg) {
    (void)arg;
    FILE *f = fopen(log_path, "w");
    if (!f) return NULL;
    fprintf(f, "timestamp_ms,vmrss_kb,pss_kb,cpu_pct\n");

    while (1) {
        sleep(5);
        long vmrss, pss;
        double cpu;
        read_proc_mem(&vmrss, &pss);
        read_proc_cpu(&cpu);

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        long long ms = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        fprintf(f, "%lld,%ld,%ld,%.2f\n", ms, vmrss, pss, cpu);
        fflush(f);
    }
    return NULL;
}

void start_monitor(const char *logfile) {
    log_path = logfile;
    pthread_t tid;
    pthread_create(&tid, NULL, monitor_loop, NULL);
    pthread_detach(tid);
}

long long get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}