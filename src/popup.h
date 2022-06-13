#pragma once
#include "background.h"
#include "misc/helpers.h"
#include "window.h"

struct bar_item;

struct popup {
  bool drawing;
  bool horizontal;
  bool overrides_cell_size;
  bool mouse_over;
  bool needs_ordering;

  char align;

  uint32_t adid;
  uint32_t cell_size;
  int y_offset;

  CGPoint anchor;
  struct window window;

  struct bar_item* host;
  struct bar_item** items;
  uint32_t num_items;

  struct background background;
};

void popup_init(struct popup* popup, struct bar_item* host);
void popup_set_anchor(struct popup* popup, CGPoint anchor, uint32_t adid);
void popup_add_item(struct popup* popup, struct bar_item* item);
bool popup_set_drawing(struct popup* popup, bool drawing);
void popup_remove_item(struct popup* popup, struct bar_item* bar_item);

uint32_t popup_get_width(struct popup* popup);
void popup_calculate_bounds(struct popup* popup);
void popup_draw(struct popup* popup);
void popup_destroy(struct popup* popup);

bool popup_parse_sub_domain(struct popup* popup, FILE* rsp, struct token property, char* message);
