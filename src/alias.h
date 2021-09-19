#ifndef ALIAS_H
#define ALIAS_H

#define MENUBAR_LAYER 0x19

struct alias {
  bool using_light_colors;
  char* name;
  uint32_t wid;
  CGImageRef image_ref;
  CGPoint size;
};

void alias_init(struct alias* alias, char* name);
bool alias_update_image(struct alias* alias);
void alias_find_window(struct alias* alias);

#endif
