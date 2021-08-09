#include "bar_item.h"

struct bar_item* bar_item_create() {
  struct bar_item* bar_item = malloc(sizeof(struct bar_item));
  memset(bar_item, 0, sizeof(struct bar_item));
  return bar_item;
}

void bar_item_init(struct bar_item* bar_item) {
  bar_item->name = "";
  bar_item->update_frequency = 5;
  bar_item->script = "";
  bar_item->position = 'r';
  bar_item->padding_left = 5;
  bar_item->padding_right = 5;
  bar_item_set_icon_font(bar_item, string_copy("Hack Nerd Font:Bold:14.0"));
  bar_item_set_icon(bar_item, string_copy(""));
  bar_item->switch_icon_side = false;
  bar_item->icon_spacing_left = 2;
  bar_item->icon_spacing_right = 2;
  bar_item->icon_color = rgba_color_from_hex(0xffffffff);
  bar_item_set_label_font(bar_item, string_copy("Hack Nerd Font:Bold:14.0"));
  bar_item_set_label(bar_item, string_copy(""));
  bar_item->label_spacing_left = 2;
  bar_item->label_spacing_right = 2;
  bar_item->label_color = rgba_color_from_hex(0xffffffff);
}

void bar_item_set_name(struct bar_item* bar_item, char* name) {
  if (name != bar_item->name && !bar_item->name) {
    free(bar_item->name);
  }
  bar_item->name = name;
}

void bar_item_set_script(struct bar_item* bar_item, char* script) {
  if (script != bar_item->script && !bar_item->script) {
    free(bar_item->script);
  }
  bar_item->script = script;
}

void bar_item_set_icon(struct bar_item* bar_item, char* icon) {
  if (bar_item->icon_line.line) {
    bar_destroy_line(bar_item->icon_line);
  }
  if (icon != bar_item->icon && !bar_item->icon) {
    free(bar_item->icon);
  }
  bar_item->icon = icon;
  bar_item->icon_line = bar_prepare_line(bar_item->icon_font, bar_item->icon, bar_item->icon_color);
}

void bar_item_set_icon_color(struct bar_item* bar_item, uint32_t color) {
  bar_item->icon_color = rgba_color_from_hex(color);
  bar_item_set_icon(bar_item, bar_item->icon);
}

void bar_item_set_label(struct bar_item* bar_item, char* label) {
  if (bar_item->label_line.line) {
    bar_destroy_line(bar_item->label_line);
  }
  if (label != bar_item->label && !bar_item->label) {
    free(bar_item->label);
  }
  bar_item->label = label;
  bar_item->label_line = bar_prepare_line(bar_item->label_font, bar_item->label, bar_item->label_color);
} 

void bar_item_set_label_color(struct bar_item* bar_item, uint32_t color) {
  bar_item->label_color = rgba_color_from_hex(color);
  bar_item_set_label(bar_item, bar_item->label);
}
void bar_item_set_icon_font(struct bar_item* bar_item, char *font_string) {
  if (bar_item->icon_font) {
    CFRelease(bar_item->icon_font);
  }

  bar_item->icon_font = bar_create_font(font_string);
}

void bar_item_set_label_font(struct bar_item* bar_item, char *font_string) {
  if (bar_item->label_font) {
    CFRelease(bar_item->label_font);
  }

  bar_item->label_font = bar_create_font(font_string);
}
