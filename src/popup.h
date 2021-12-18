#ifndef POPUP_H
#define POPUP_H

#include <_types/_uint32_t.h>
struct popup {
  uint32_t id;
  uint32_t adid;
  bool drawing;
  bool horizontal;
  int y_offset;
  uint32_t cell_size;
  CGPoint anchor;
  CGContextRef context;
  CGRect frame;
  struct background background;

  uint32_t num_items;
  struct bar_item** items;
};

void popup_init(struct popup* popup);
void popup_set_anchor(struct popup* popup, CGPoint anchor, uint32_t adid);
void popup_add_item(struct popup* popup, struct bar_item* item);
void popup_set_drawing(struct popup* popup, bool drawing);
void popup_draw(struct popup* popup);

void popup_destroy(struct popup* popup);

static bool popup_parse_sub_domain(struct popup* popup, FILE* rsp, struct token property, char* message);

#endif // !POPUP_H
