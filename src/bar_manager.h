#ifndef BAR_MANAGER_H
#define BAR_MANAGER_H

struct bar_manager
{
  struct bar **bars;
  int bar_count;
  struct bar_item **bar_items;
  int bar_item_count;
  char *position;
  char *display;
  uint32_t height;
  uint32_t padding_left;
  uint32_t padding_right;
  bool title;
  bool spaces;
  struct rgba_color foreground_color;
  struct rgba_color background_color;
  struct rgba_color space_icon_color;
  struct rgba_color background_color_dim;
};

int bar_manager_get_item_index_for_name(struct bar_manager* bar_manager, char* name);
struct bar_item* bar_manager_create_item(struct bar_manager* bar_manager);

void bar_manager_update_components(struct bar_manager* bar_manager, uint32_t did, uint32_t sid);
void bar_manager_set_foreground_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_background_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_space_icon_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_space_icon_color_secondary(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_space_icon_color_tertiary(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_display_separator_icon_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_text_font(struct bar_manager *bar_manager, char *font_string);
void bar_manager_set_icon_font(struct bar_manager *bar_manager, char *font_string);
void bar_manager_set_space_strip(struct bar_manager *bar_manager, char **icon_strip);
void bar_manager_set_space_icon(struct bar_manager *bar_manager, char *icon);
void bar_manager_set_display_separator(struct bar_manager *bar_manager, bool value);
void bar_manager_set_display_separator_icon(struct bar_manager *bar_manager, char *icon);
void bar_manager_set_position(struct bar_manager *bar_manager, char *pos);
void bar_manager_set_title(struct bar_manager *bar_manager, bool value);
void bar_manager_set_spaces(struct bar_manager *bar_manager, bool value);
void bar_manager_set_spaces_for_all_displays(struct bar_manager *bar_manager, bool value);
void bar_manager_set_height(struct bar_manager *bar_manager, uint32_t height);
void bar_manager_set_padding_left(struct bar_manager *bar_manager, uint32_t padding);
void bar_manager_set_padding_right(struct bar_manager *bar_manager, uint32_t padding);
void bar_manager_set_display(struct bar_manager *bar_manager, char *display);

void bar_manager_display_changed(struct bar_manager *bar_manager);
void bar_manager_refresh(struct bar_manager *bar_manager);
void bar_manager_resize(struct bar_manager *bar_manager);
void bar_manager_begin(struct bar_manager *bar_manager);
void bar_manager_init(struct bar_manager *bar_manager);

#endif
