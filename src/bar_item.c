#include "bar_item.h"

struct bar_item* bar_item_create() {
  struct bar_item* bar_item = malloc(sizeof(struct bar_item));
  memset(bar_item, 0, sizeof(struct bar_item));
  return bar_item;
}

void bar_item_init(struct bar_item* bar_item) {
  bar_item->nospace = false;
  bar_item->counter = 0;
  bar_item->name = "";
  bar_item->type = BAR_ITEM;
  bar_item->update_frequency = 1000;
  bar_item->script = "";
  bar_item->on_click_script = "~/.config/sketchybar/plugins/random.sh";
  bar_item->position = BAR_POSITION_RIGHT;
  bar_item->associated_display = 0;
  bar_item->associated_space = 0;
  bar_item->icon_spacing_left = 0;
  bar_item->icon_spacing_right = 0;
  bar_item->icon_color = rgba_color_from_hex(0xffffffff);
  bar_item->icon_highlight_color = rgba_color_from_hex(0xffffffff);
  bar_item->label_spacing_left = 0;
  bar_item->label_spacing_right = 0;
  bar_item->label_color = rgba_color_from_hex(0xffffffff);
  bar_item->has_graph = false;
  bar_item->num_rects = 0;
  bar_item->bounding_rects = NULL;

  bar_item_set_icon(bar_item, string_copy(""), bar_item->icon_color);
  bar_item_set_icon_font(bar_item, string_copy("Hack Nerd Font:Bold:14.0"));
  bar_item_set_label_font(bar_item, string_copy("Hack Nerd Font:Bold:14.0"));
  bar_item_set_label(bar_item, string_copy(""));
}

void bar_item_script_update(struct bar_item* bar_item, bool forced) {
  if (strcmp(bar_item->script, "") != 0) {
    bar_item->counter++;
    if (bar_item->update_frequency < bar_item->counter || forced) { 
      bar_item->counter = 0; 
      fork_exec(bar_item->script, NULL);
    }
  }
}

void bar_item_update_component(struct bar_item* bar_item, uint32_t did, uint32_t sid) {
  if (bar_item->type == BAR_COMPONENT) {
    if (strcmp(bar_item->identifier, BAR_COMPONENT_TITLE) == 0)
      bar_item_set_label(bar_item, focused_window_title());
    else if (strcmp(bar_item->identifier, BAR_COMPONENT_SPACE) == 0) {
      if ((1 << sid) & bar_item->associated_space && (1 << did) & bar_item->associated_display)
        bar_item_set_icon(bar_item, bar_item->icon, bar_item->icon_highlight_color);
      else 
        bar_item_set_icon(bar_item, bar_item->icon, bar_item->icon_color);
      
    }
  }
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

void bar_item_set_click_script(struct bar_item* bar_item, char* script) {
  if (script != bar_item->on_click_script && !bar_item->on_click_script) {
    free(bar_item->on_click_script);
  }
  bar_item->on_click_script = script;
}

void bar_item_set_icon(struct bar_item* bar_item, char* icon, struct rgba_color color) {
  if (bar_item->icon_line.line) {
    bar_destroy_line(bar_item->icon_line);
  }
  if (icon != bar_item->icon && !bar_item->icon) {
    free(bar_item->icon);
  }
  bar_item->icon = icon;
  bar_item->icon_line = bar_prepare_line(bar_item->icon_font, bar_item->icon, color);
}

void bar_item_set_icon_color(struct bar_item* bar_item, uint32_t color) {
  bar_item->icon_color = rgba_color_from_hex(color);
  bar_item_set_icon(bar_item, bar_item->icon, bar_item->icon_color);
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

void bar_item_on_click(struct bar_item* bar_item) {
  printf("%s \n", bar_item->name);
  if (strcmp(bar_item->on_click_script, "") != 0)
    fork_exec(bar_item->on_click_script, NULL);
}

CGRect bar_item_construct_bounding_rect(struct bar_item* bar_item) {
  CGRect bounding_rect;
  bounding_rect.origin = bar_item->icon_line.bounds.origin;
  bounding_rect.size.width = CGRectGetMaxX(bar_item->label_line.bounds) - CGRectGetMinX(bar_item->icon_line.bounds);
  bounding_rect.size.height = CGRectGetMaxY(bar_item->label_line.bounds) > CGRectGetMaxY(bar_item->icon_line.bounds) ? CGRectGetMaxY(bar_item->label_line.bounds) : CGRectGetMaxY(bar_item->icon_line.bounds);
  return bounding_rect;
}

void bar_item_set_bounding_rect_for_space(struct bar_item* bar_item, uint32_t sid) {
  if (bar_item->num_rects < sid) {
    bar_item->bounding_rects = (CGRect**) realloc(bar_item->bounding_rects, sizeof(CGRect*) * sid);
    memset(bar_item->bounding_rects, 0, sizeof(CGRect*) * sid);
    bar_item->num_rects = sid;
  }
  if (!bar_item->bounding_rects[sid - 1]) {
    bar_item->bounding_rects[sid - 1] = malloc(sizeof(CGRect));
    memset(bar_item->bounding_rects[sid - 1], 0, sizeof(CGRect));
  }
  CGRect rect = bar_item_construct_bounding_rect(bar_item);

  bar_item->bounding_rects[sid - 1]->origin = rect.origin;
  bar_item->bounding_rects[sid - 1]->size = rect.size;
}
