#pragma once
#include "text.h"
#include "window.h"

struct bar_item;

struct tooltip {
  struct bar_item* host;
  CGRect anchor;

  struct text text;
  struct window window;
};

void tooltip_init(struct tooltip* tooltip);
void tooltip_show(struct tooltip* tooltip, struct bar_item* host, CGRect anchor);
void tooltip_hide(struct tooltip* tooltip);
