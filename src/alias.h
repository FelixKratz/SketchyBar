#ifndef ALIAS_H
#define ALIAS_H

#include <stdint.h>
#define MENUBAR_LAYER 0x19

struct alias {
  bool using_light_colors;
  bool permission;
  char* name;
  char* owner;
  uint64_t pid;
  uint32_t wid;
  CGImageRef image_ref;
  CGRect bounds;
};

void print_all_menu_items(FILE* rsp);
void alias_init(struct alias* alias, char* owner, char* name);
bool alias_update_image(struct alias* alias);
void alias_find_window(struct alias* alias);
uint32_t alias_get_length(struct alias* alias);
uint32_t alias_get_height(struct alias* alias);

void alias_calculate_bounds(struct alias* alias, uint32_t x, uint32_t y);
void alias_draw(struct alias* alias, CGContextRef context);

#endif
