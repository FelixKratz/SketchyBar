// Thanks to github user: jkorinth 
#include "cpu.h"

static void cpu_refresh_handler(CFRunLoopTimerRef timer, void *ctx)
{
  assert(ctx);
  if (ctx) {
    cpu_update((struct cpu_info *) ctx);
  }
}

void cpu_start_update(struct cpu_info* cpui)
{
  assert("cpui is not NULL" && cpui);
  // setup timer to update values
  CFRunLoopTimerContext ctx = {
    .version = 0,
    .info = cpui,
    .copyDescription = NULL,
    .retain = NULL,
    .release = NULL,
  };
  cpui->update_freq = cpui->update_freq > 0.0 ? cpui->update_freq : 1.0;
  cpui->refresh_timer = CFRunLoopTimerCreate(
      NULL,
      CFAbsoluteTimeGetCurrent() + cpui->update_freq,
      cpui->update_freq,
      0,
      0,
      cpu_refresh_handler,
      &ctx
      );
  CFRunLoopAddTimer(CFRunLoopGetMain(), cpui->refresh_timer, kCFRunLoopCommonModes);
  cpui->is_running = true;
}

void cpu_stop_update(struct cpu_info* cpui)
{
  assert("cpui is not NULL" && cpui);
  CFRunLoopRemoveTimer(CFRunLoopGetMain(), cpui->refresh_timer, kCFRunLoopCommonModes);
  CFRunLoopTimerInvalidate(cpui->refresh_timer);
}

void cpu_set_update_frequency(struct cpu_info* cpui, float seconds)
{
  assert("cpui is not NULL" && cpui);
  bool was_running = cpui->is_running;
  cpui->update_freq = seconds;
  if (was_running) {
    cpu_stop_update(cpui);
    cpu_start_update(cpui);
  }
}

void cpu_create(struct cpu_info* cpui)
{
  assert("cpui is not NULL" && cpui);
  char tmp[255];
  size_t len = sizeof(cpui->nphys_cpu);
  if (sysctlbyname("hw.physicalcpu", &cpui->nphys_cpu, &len, NULL, 0)) {
    snprintf(tmp, sizeof(tmp), "error! could not retrieve hw.physicalcpu: %s", strerror(errno));
  }
  cpu_start_update(cpui);
}

void cpu_update(struct cpu_info* cpui)
{
  assert("cpui is not NULL" && cpui);
  mach_msg_type_number_t info_size = sizeof(processor_cpu_load_info_t);
  if (cpui->prev_load) {
    munmap(cpui->prev_load, vm_page_size);
  }
  cpui->prev_load = cpui->curr_load;
  if (host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &cpui->nlog_cpu,
        (processor_info_array_t *)&cpui->curr_load, &info_size)) {
  }

  // shift buffers to the left
  memmove(cpui->load_avg, &cpui->load_avg[1], sizeof(*cpui->load_avg) * (CPU_WINDOW_SZ - 1));
  memmove(cpui->sys_avg, &cpui->sys_avg[1], sizeof(*cpui->sys_avg) * (CPU_WINDOW_SZ - 1));
  memmove(cpui->user_avg, &cpui->user_avg[1], sizeof(*cpui->user_avg) * (CPU_WINDOW_SZ - 1));
  memmove(cpui->used_mem, &cpui->used_mem[1], sizeof(*cpui->used_mem) * (CPU_WINDOW_SZ - 1));
  cpui->user_avg[CPU_WINDOW_SZ - 1] = 0;
  cpui->sys_avg[CPU_WINDOW_SZ - 1] = 0;
  cpui->load_avg[CPU_WINDOW_SZ - 1] = 0;
  if (cpui->prev_load) {
    for (size_t cpu = 0; cpu < cpui->nlog_cpu; ++cpu) {
      double total_ticks = 0.0;
      for (size_t s = 0; s < CPU_STATE_MAX; ++s) {
        total_ticks += cpui->curr_load[cpu].cpu_ticks[s] - cpui->prev_load[cpu].cpu_ticks[s];
      }
      cpui->user_avg[CPU_WINDOW_SZ - 1] +=
        (cpui->curr_load[cpu].cpu_ticks[CPU_STATE_USER] -
         cpui->prev_load[cpu].cpu_ticks[CPU_STATE_USER]) / total_ticks;
      cpui->sys_avg[CPU_WINDOW_SZ - 1] +=
        (cpui->curr_load[cpu].cpu_ticks[CPU_STATE_SYSTEM] -
         cpui->prev_load[cpu].cpu_ticks[CPU_STATE_SYSTEM]) / total_ticks;
      cpui->load_avg[CPU_WINDOW_SZ - 1] +=
        (cpui->curr_load[cpu].cpu_ticks[CPU_STATE_USER] -
         cpui->prev_load[cpu].cpu_ticks[CPU_STATE_USER]) / total_ticks +
        (cpui->curr_load[cpu].cpu_ticks[CPU_STATE_SYSTEM] -
         cpui->prev_load[cpu].cpu_ticks[CPU_STATE_SYSTEM]) / total_ticks;
    }
  }
  cpui->user_avg[CPU_WINDOW_SZ - 1] /= (float)cpui->nlog_cpu;
  cpui->sys_avg[CPU_WINDOW_SZ - 1] /= (float)cpui->nlog_cpu;
  cpui->load_avg[CPU_WINDOW_SZ - 1] /= (float)cpui->nlog_cpu;

  vm_size_t page_size;
  mach_port_t mach_port;
  mach_msg_type_number_t count;
  vm_statistics64_data_t vm_stats;

  mach_port = mach_host_self();
  count = sizeof(vm_stats) / sizeof(natural_t);
  if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
      KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
                                      (host_info64_t)&vm_stats, &count)) {
      long long free_memory = (int64_t)vm_stats.free_count * (int64_t)page_size;

      long long used_memory = ((int64_t)vm_stats.active_count +
                               (int64_t)vm_stats.inactive_count +
                               (int64_t)vm_stats.wire_count) *  (int64_t)page_size;
      cpui->used_mem[CPU_WINDOW_SZ - 1] = (double)used_memory / (double)(free_memory + used_memory);
  }
}

void cpu_destroy(struct cpu_info* cpui)
{
  assert("cpui is not NULL" && cpui);
  cpu_stop_update(cpui);
}
