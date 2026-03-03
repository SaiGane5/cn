#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "monitor.h"

static const char *log_path;

static void read_proc_mem(long *vmrss_kb, long *pss_kb) {
    *vmrss_kb = 0; *pss_kb = 0;
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
}

static void read_proc_cpu(double *cpu_pct) {
    static long prev_utime = 0, prev_stime = 0;
    static long long prev_wall = 0;
    *cpu_pct = 0.0;

    FILE *f = fopen("/proc/self/stat", "r");
    if (!f) return;
    long utime, stime;
    int dummy_i; char dummy_c; long dummy_l;
    fscanf(f, "%d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld",
           &dummy_i, &dummy_c, &utime, &stime);
    fclose(f);

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long wall_ms = (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    long hz = sysconf(_SC_CLK_TCK);
    long delta_cpu  = (utime + stime) - (prev_utime + prev_stime);
    long long delta_wall = wall_ms - prev_wall;

    if (prev_wall > 0 && delta_wall > 0)
        *cpu_pct = 100.0 * ((double)delta_cpu / hz) / ((double)delta_wall / 1000.0);

    prev_utime = utime; prev_stime = stime; prev_wall = wall_ms;
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