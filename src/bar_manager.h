#ifndef BAR_MANAGER_H
#define BAR_MANAGER_H

struct bar_manager
{
  struct bar **bars;
  int bar_count;
  struct bar_item **bar_items;
  int bar_item_count;
  char *t_font_prop;
  char *i_font_prop;
  CTFontRef t_font;
  CTFontRef i_font;
  char **_space_icon_strip;
  char **_power_icon_strip;
  char *_space_icon;
  char *position;
  char *display;
  char *_display_separator_icon;
  uint32_t height;
  uint32_t padding_left;
  uint32_t padding_right;
  bool title;
  bool spaces;
  bool spaces_for_all_displays;
  bool display_separator;
  struct rgba_color foreground_color;
  struct rgba_color background_color;
  struct rgba_color space_icon_color;
  struct rgba_color space_icon_color_secondary;
  struct rgba_color space_icon_color_tertiary;
  struct rgba_color display_separator_icon_color;
  struct rgba_color background_color_dim;
  struct bar_line *space_icon_strip;
  struct bar_line space_icon;
  struct bar_line display_separator_icon;
};

int bar_manager_get_item_index_for_name(struct bar_manager* bar_manager, char* name);

void bar_manager_set_foreground_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_background_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_space_icon_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_space_icon_color_secondary(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_space_icon_color_tertiary(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_display_separator_icon_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_text_font(struct bar_manager *bar_manager, char *font_string);
void bar_manager_set_icon_font(struct bar_manager *bar_manager, char *font_string);
void bar_manager_set_space_strip(struct bar_manager *bar_manager, char **icon_strip);
void bar_manager_set_power_strip(struct bar_manager *bar_manager, char **icon_strip);
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
