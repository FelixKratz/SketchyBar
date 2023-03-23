#pragma once
#include "bar_item.h"
#include "misc/helpers.h"
#include "window.h"

struct bar {
  bool shown;
  bool hidden;
  bool mouse_over;

  uint32_t sid;
  uint32_t dsid;
  uint32_t did;
  uint32_t adid;

  struct window window;
};

struct bar *bar_create(uint32_t did);
void bar_close_window(struct bar* bar);
void bar_destroy(struct bar* bar);
void bar_set_hidden(struct bar* bar, bool hidden);
void bar_calculate_bounds(struct bar* bar);
void bar_resize(struct bar* bar);
void bar_draw(struct bar* bar, bool forced);
void bar_order_item_windows(struct bar* bar);

bool bar_draws_item(struct bar* bar, struct bar_item* bar_item);

void bar_change_space(struct bar* bar, uint64_t dsid);

void context_set_font_smoothing(CGContextRef context, bool smoothing);
