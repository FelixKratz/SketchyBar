#pragma once
#include "event.h"

struct app_window {
  uint32_t wid;
  uint64_t sid;
  pid_t pid;
};

struct app_windows {
  struct app_window* windows;
  uint32_t num_windows;
};

void begin_receiving_space_window_events();
void forced_space_windows_event();
