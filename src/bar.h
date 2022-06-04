#pragma once
#include "bar_item.h"
#include "misc/helpers.h"
#include "window.h"

#define ALIGN_NONE   0
#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2
#define ALIGN_TOP    3
#define ALIGN_BOTTOM 4
#define ALIGN_CENTER 5

struct bar {
  bool needs_update;
  bool shown;
  bool hidden;

  uint32_t did;
  uint32_t sid;
  uint32_t adid;

  struct window window;
};

struct bar *bar_create(uint32_t did);
void bar_create_window(struct bar* bar);
void bar_close_window(struct bar* bar);
void bar_destroy(struct bar* bar);
void bar_set_hidden(struct bar* bar, bool hidden);
void bar_calculate_bounds(struct bar* bar);
void bar_resize(struct bar* bar);
void bar_draw(struct bar* bar);

bool bar_draws_item(struct bar* bar, struct bar_item* bar_item);

void context_set_font_smoothing(CGContextRef context, bool smoothing);
