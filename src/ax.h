#pragma once
#include <Carbon/Carbon.h>

struct ax {
  bool is_privileged;

  uint32_t element_count;
  AXUIElementRef* elements;
};

struct ax g_ax;
void ax_init(struct ax* ax, bool check_privileges);
void ax_clear(struct ax* ax);
void ax_get_menu_item(struct ax* ax, pid_t pid, char* name);
