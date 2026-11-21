#pragma once
#include <CoreGraphics/CoreGraphics.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// Starting with macOS 27 the status items of all apps are drawn by the
// MenuBarAgent process into a single window per display. The items no longer
// have windows of their own, so they can not be found via the window list.
// Instead, the items are read from the accessibility tree of the MenuBarAgent
// window: every child of that window is one item, and the first element below
// it that does not belong to MenuBarAgent belongs to the app owning the item.

struct menubar_agent_item {
  char* owner;
  char* name;
  pid_t pid;

  // Window id of the MenuBarAgent window that draws the item
  uint32_t wid;
  // Frame of the item in global screen coordinates
  CGRect frame;
  // Frame of the item relative to the MenuBarAgent window
  CGRect crop;
};

bool menubar_agent_available(void);
bool menubar_agent_trusted(bool prompt);

// Returns the items currently drawn on the menu bar of the main display.
// The caller owns the returned array and has to release it with
// menubar_agent_items_free.
uint32_t menubar_agent_copy_items(struct menubar_agent_item** items);
void menubar_agent_items_free(struct menubar_agent_item* items, uint32_t count);

// Presses the item drawn at frame (global screen coordinates) via the
// accessibility API, which opens its menu.
bool menubar_agent_press(CGRect frame);
