#ifndef BAR_ITEM_H
#define BAR_ITEM_H

struct bar_item {
  uint32_t update_frequency;
  // Execute with exec_fork, callback from command via messages
  char* script;
  
  // Name by which to refer to the bar_item in the configuration
  char* name;

  // The position in the bar: l,r,c
  char position;

  // Item padding setup
  uint32_t padding_left;
  uint32_t padding_right;

  // Icon properties
  struct bar_line icon_line;
  char* icon;
  CTFontRef icon_font;
  bool switch_icon_side; // true: left, false: right
  uint32_t icon_spacing_left;
  uint32_t icon_spacing_right;
  struct rgba_color icon_color;

  // Label properties
  struct bar_line label_line;
  char* label;
  CTFontRef label_font;
  uint32_t label_spacing_left;
  uint32_t label_spacing_right;
  struct rgba_color label_color;

  // Seperator setup
  struct bar_line separator_line;
  char* separator_left;
  struct rgba_color separator_left_color;
  char* separator_right;
  struct rgba_color separator_right_color;
};

struct bar_item* bar_item_create();
void bar_item_init(struct bar_item* bar_item);
void bar_item_set_name(struct bar_item* bar_item, char* name);
void bar_item_set_script(struct bar_item* bar_item, char* script);
void bar_item_set_padding_left(struct bar_item* bar_item, uint32_t pad);
void bar_item_set_padding_right(struct bar_item* bar_item, uint32_t pad);
void bar_item_set_icon(struct bar_item* bar_item, char* icon);
void bar_item_set_icon_color(struct bar_item* bar_item, uint32_t color);
void bar_item_set_label(struct bar_item* bar_item, char* label);
void bar_item_set_label_color(struct bar_item* bar_item, uint32_t color);
void bar_item_set_separator_left(struct bar_item* bar_item, char* sep);
void bar_item_set_separator_right(struct bar_item* bar_item, char* sep);
void bar_item_set_label_font(struct bar_item* bar_item, char *font_string);
void bar_item_set_icon_font(struct bar_item* bar_item, char *font_string);



#endif
