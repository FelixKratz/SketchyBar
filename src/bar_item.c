#include "bar_item.h"
#include "graph_data.h"
#include "misc/helpers.h"
#include <string.h>

struct bar_item* bar_item_create() {
  struct bar_item* bar_item = malloc(sizeof(struct bar_item));
  memset(bar_item, 0, sizeof(struct bar_item));
  return bar_item;
}

void bar_item_init(struct bar_item* bar_item, struct bar_item* default_item) {
  bar_item->needs_update = true;
  bar_item->lazy = false;
  bar_item->drawing = true;
  bar_item->scripting = true;
  bar_item->is_shown = false;
  bar_item->nospace = false;
  bar_item->selected = false;
  bar_item->counter = 0;
  bar_item->name = "";
  bar_item->type = BAR_ITEM;
  bar_item->identifier = "";
  bar_item->update_frequency = 0;
  bar_item->cache_scripts = false;
  bar_item->script = "";
  bar_item->click_script = "";
  bar_item->position = BAR_POSITION_RIGHT;
  bar_item->associated_display = 0;
  bar_item->associated_space = 0;
  bar_item->icon_font_name = "Hack Nerd Font:Bold:14.0";
  bar_item->label_font_name = "Hack Nerd Font:Bold:14.0";
  bar_item->icon_highlight = false;
  bar_item->icon = "";
  bar_item->icon_spacing_left = 0;
  bar_item->icon_spacing_right = 0;
  bar_item->icon_color = rgba_color_from_hex(0xffffffff);
  bar_item->icon_highlight_color = rgba_color_from_hex(0xffffffff);
  bar_item->label_highlight = false;
  bar_item->label = "";
  bar_item->label_spacing_left = 0;
  bar_item->label_spacing_right = 0;
  bar_item->label_color = rgba_color_from_hex(0xffffffff);
  bar_item->label_highlight_color = rgba_color_from_hex(0xffffffff);
  bar_item->has_graph = false;
  bar_item->num_rects = 0;
  bar_item->background_color = rgba_color_from_hex(0x44ff0000);
  bar_item->bounding_rects = NULL;

  if (default_item) {
    bar_item->scripting = default_item->scripting;
    bar_item->drawing = default_item->drawing;
    bar_item->icon_color = default_item->icon_color;
    bar_item->icon_font_name = default_item->icon_font_name;
    bar_item->label_color = default_item->label_color;
    bar_item->label_font_name = default_item->label_font_name;
    bar_item->icon_spacing_left = default_item->icon_spacing_left;
    bar_item->icon_spacing_right = default_item->icon_spacing_right;
    bar_item->label_spacing_left = default_item->label_spacing_left;
    bar_item->label_spacing_right = default_item->label_spacing_right;
    bar_item->update_frequency = default_item->update_frequency;
    bar_item->cache_scripts = default_item->cache_scripts;
    bar_item->icon_highlight_color = default_item->icon_highlight_color;
    bar_item->label_highlight_color = default_item->label_highlight_color;
  }

  bar_item_set_icon(bar_item, string_copy(""), false);
  bar_item_set_icon_font(bar_item, string_copy(bar_item->icon_font_name), true);
  bar_item_set_label_font(bar_item, string_copy(bar_item->label_font_name), true);
  bar_item_set_label(bar_item, string_copy(""), false);

  strncpy(&bar_item->signal_args.name[0][0], "NAME", 255);
  strncpy(&bar_item->signal_args.name[1][0], "SELECTED", 255);
  strncpy(&bar_item->signal_args.value[1][0], "false", 255);
}

void bar_item_script_update(struct bar_item* bar_item, bool forced) {
  if (!bar_item->scripting || (bar_item->update_frequency == 0 && !forced)) return;
  if (strlen(bar_item->script) > 0) {
    bar_item->counter++;
    if (bar_item->update_frequency <= bar_item->counter || forced) {
      bar_item->counter = 0;
      fork_exec(bar_item->script, &bar_item->signal_args);
    }
  }
}

void bar_item_needs_update(struct bar_item* bar_item) {
  bar_item->needs_update = true;
}

void bar_item_clear_needs_update(struct bar_item* bar_item) {
  bar_item->needs_update = false;
}

void bar_item_set_name(struct bar_item* bar_item, char* name) {
  if (!name) return;
  if (bar_item->name && strcmp(bar_item->name, name) == 0) { free(name); return; }
  if (name != bar_item->name && !bar_item->name) free(bar_item->name);
  bar_item->name = name;
  strncpy(&bar_item->signal_args.value[0][0], name, 255);
}

void bar_item_set_script(struct bar_item* bar_item, char* script) {
  if (!script) return;
  if (bar_item->script && strcmp(bar_item->script, script) == 0) { free(script); return; }
  if (script != bar_item->script && !bar_item->script) free(bar_item->script);
  if (bar_item->cache_scripts && file_exists(resolve_path(script))) bar_item->script = read_file(resolve_path(script));
  else bar_item->script = script;
}

void bar_item_set_click_script(struct bar_item* bar_item, char* script) {
  if (!script) return;
  if (bar_item->click_script && strcmp(bar_item->click_script, script) == 0) { free(script); return; }
  if (script != bar_item->click_script && !bar_item->click_script) free(bar_item->click_script);
  if (bar_item->cache_scripts && file_exists(resolve_path(script))) bar_item->click_script = read_file(resolve_path(script));
  else bar_item->click_script = script;
}

void bar_item_set_icon(struct bar_item* bar_item, char* icon, bool forced) {
  if (!icon) return;
  if (!forced && bar_item->icon && strcmp(bar_item->icon, icon) == 0) { free(icon); return; }
  if (bar_item->icon_line.line) bar_destroy_line(&bar_item->icon_line);
  if (icon != bar_item->icon && !bar_item->icon) free(bar_item->icon);
  bar_item->icon = icon;
  bar_prepare_line(&bar_item->icon_line, bar_item->icon_font, bar_item->icon, bar_item->icon_highlight ? bar_item->icon_highlight_color : bar_item->icon_color);
  bar_item_needs_update(bar_item);
}

void bar_item_update_icon_color(struct bar_item *bar_item) {
  struct rgba_color target_color = bar_item->icon_highlight ? bar_item->icon_highlight_color : bar_item->icon_color;
  if (bar_item->icon_line.color.r == target_color.r 
      && bar_item->icon_line.color.g == target_color.g 
      && bar_item->icon_line.color.b == target_color.b 
      && bar_item->icon_line.color.a == target_color.a) return;
  bar_item->icon_line.color = target_color;
  bar_item_needs_update(bar_item);
}

void bar_item_set_icon_color(struct bar_item* bar_item, uint32_t color) {
  bar_item->icon_color = rgba_color_from_hex(color);
  bar_item_update_icon_color(bar_item);
}

void bar_item_set_label(struct bar_item* bar_item, char* label, bool forced) {
  if (!label) return;
  if (!forced && bar_item->label && strcmp(bar_item->label, label) == 0) { free(label); return; }
  if (bar_item->label_line.line) bar_destroy_line(&bar_item->label_line);
  if (label != bar_item->label && !bar_item->label) free(bar_item->label);
  bar_item->label = label;
  bar_prepare_line(&bar_item->label_line, bar_item->label_font, bar_item->label, bar_item->label_highlight ? bar_item->label_highlight_color : bar_item->label_color);
  bar_item_needs_update(bar_item);
} 


void bar_item_set_label_color(struct bar_item* bar_item, uint32_t color) {
  bar_item->label_color = rgba_color_from_hex(color);
  bar_item_update_label_color(bar_item);
}

void bar_item_update_label_color(struct bar_item *bar_item) {
  struct rgba_color target_color = bar_item->label_highlight ? bar_item->label_highlight_color : bar_item->label_color;
  if (bar_item->label_line.color.r == target_color.r 
      && bar_item->label_line.color.g == target_color.g 
      && bar_item->label_line.color.b == target_color.b 
      && bar_item->label_line.color.a == target_color.a) return;
  bar_item->label_line.color = target_color;
  bar_item_needs_update(bar_item);
}

void bar_item_set_icon_font(struct bar_item* bar_item, char *font_string, bool forced) {
  if (!font_string) return;
  if (!forced && bar_item->icon_font_name && strcmp(bar_item->icon_font_name, font_string) == 0) { free(font_string); return; }
  if (bar_item->icon_font) CFRelease(bar_item->icon_font);

  bar_item->icon_font = bar_create_font(font_string);
  bar_item->icon_font_name = font_string;
  bar_item_set_icon(bar_item, bar_item->icon, true);
}

void bar_item_set_label_font(struct bar_item* bar_item, char *font_string, bool forced) {
  if (!font_string) return;
  if (!forced && bar_item->label_font_name && strcmp(bar_item->label_font_name, font_string) == 0) { free(font_string); return; }
  if (bar_item->label_font) CFRelease(bar_item->label_font);

  bar_item->label_font = bar_create_font(font_string);
  bar_item->label_font_name = font_string;
  bar_item_set_label(bar_item, bar_item->label, true);
}

void bar_item_on_click(struct bar_item* bar_item) {
  if (!bar_item) return;
  if (bar_item && strlen(bar_item->click_script) > 0)
    fork_exec(bar_item->click_script, &bar_item->signal_args);
}

CGRect bar_item_construct_bounding_rect(struct bar_item* bar_item) {
  CGRect bounding_rect;
  bounding_rect.origin = bar_item->icon_line.bounds.origin;
  bounding_rect.origin.x -= bar_item->icon_spacing_left;
  bounding_rect.origin.y = bar_item->icon_line.bounds.origin.y < bar_item->label_line.bounds.origin.y ? bar_item->icon_line.bounds.origin.y : bar_item->label_line.bounds.origin.y;
  bounding_rect.size.width = CGRectGetMaxX(bar_item->label_line.bounds) - CGRectGetMinX(bar_item->icon_line.bounds) + bar_item->icon_spacing_left + bar_item->label_spacing_right + 1;
  uint32_t max_y = CGRectGetMaxY(bar_item->label_line.bounds) > CGRectGetMaxY(bar_item->icon_line.bounds) ? CGRectGetMaxY(bar_item->label_line.bounds) : CGRectGetMaxY(bar_item->icon_line.bounds);
  uint32_t min_y = CGRectGetMinY(bar_item->label_line.bounds) < CGRectGetMinY(bar_item->icon_line.bounds) ? CGRectGetMinY(bar_item->label_line.bounds) : CGRectGetMinY(bar_item->icon_line.bounds);
  bounding_rect.size.height = max_y - min_y;
  return bounding_rect;
}

void bar_item_set_bounding_rect_for_space(struct bar_item* bar_item, uint32_t sid, CGPoint bar_origin) {
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
  bar_item->bounding_rects[sid - 1]->origin.x = rect.origin.x + bar_origin.x;
  bar_item->bounding_rects[sid - 1]->origin.y = rect.origin.y + bar_origin.y;
  bar_item->bounding_rects[sid - 1]->size = rect.size;
}

void bar_item_destroy(struct bar_item* bar_item) {
  if (bar_item->name) free(bar_item->name);
  if (bar_item->script && !bar_item->cache_scripts) free(bar_item->script);
  if (bar_item->click_script && !bar_item->cache_scripts) free(bar_item->click_script);
  if (bar_item->icon) free(bar_item->icon);
  if (bar_item->icon_font_name) free(bar_item->icon_font_name);
  if (bar_item->label) free(bar_item->label);
  if (bar_item->label_font_name) free(bar_item->label_font_name);
  if (bar_item->identifier) free(bar_item->identifier);

  if (bar_item->bounding_rects) {  
    for (int j = 0; j < bar_item->num_rects; j++) {
      free(bar_item->bounding_rects[j]);
    }
    free(bar_item->bounding_rects);
  }
  if (bar_item->has_graph) {
    graph_data_destroy(&bar_item->graph_data);
  }
  free(bar_item);
}
