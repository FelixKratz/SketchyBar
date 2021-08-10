// Thanks to github user: jkorinth 
#ifndef CPU_H
#define CPU_H

#include <assert.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <mach/mach_host.h>

#define CPU_WINDOW_SZ                               40

struct cpu_info
{
  int32_t nphys_cpu;
  unsigned int nlog_cpu;
  float update_freq;
  bool is_running;
  float user_avg[CPU_WINDOW_SZ];
  float sys_avg[CPU_WINDOW_SZ];
  float load_avg[CPU_WINDOW_SZ];
  float used_mem[CPU_WINDOW_SZ];
  CFRunLoopTimerRef refresh_timer;
  processor_cpu_load_info_t prev_load;
  processor_cpu_load_info_t curr_load;
};

void cpu_start_update(struct cpu_info* cpui);
void cpu_stop_update(struct cpu_info* cpui);
void cpu_set_update_frequency(struct cpu_info* cpui, float seconds);

void cpu_create(struct cpu_info* cpui);
void cpu_update(struct cpu_info* cpui);
void cpu_destroy(struct cpu_info* cpui);

#endif
