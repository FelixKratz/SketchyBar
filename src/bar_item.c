#include "bar_item.h"
#include "alias.h"
#include "graph_data.h"
#include "misc/helpers.h"
#include <stdint.h>
#include <string.h>

struct bar_item* bar_item_create() {
  struct bar_item* bar_item = malloc(sizeof(struct bar_item));
  memset(bar_item, 0, sizeof(struct bar_item));
  return bar_item;
}

void bar_item_inherit_from_item(struct bar_item* bar_item, struct bar_item* ancestor) {
  bar_item->lazy = ancestor->lazy;
  bar_item->updates = ancestor->updates;
  bar_item->drawing = ancestor->drawing;
  bar_item->icon_color = ancestor->icon_color;
  bar_item->icon_font_name = ancestor->icon_font_name;
  bar_item->label_color = ancestor->label_color;
  bar_item->label_font_name = ancestor->label_font_name;
  bar_item->icon_spacing_left = ancestor->icon_spacing_left;
  bar_item->icon_spacing_right = ancestor->icon_spacing_right;
  bar_item->label_spacing_left = ancestor->label_spacing_left;
  bar_item->label_spacing_right = ancestor->label_spacing_right;
  bar_item->update_frequency = ancestor->update_frequency;
  bar_item->cache_scripts = ancestor->cache_scripts;
  bar_item->icon_highlight_color = ancestor->icon_highlight_color;
  bar_item->label_highlight_color = ancestor->label_highlight_color;
  bar_item->background_color = ancestor->background_color;
  bar_item->draws_background = ancestor->draws_background;
  bar_item->background_height = ancestor->background_height;
  bar_item->background_corner_radius = ancestor->background_corner_radius;
  bar_item->background_border_color = ancestor->background_border_color;
  bar_item->background_border_width = ancestor->background_border_width;
  bar_item->y_offset = ancestor->y_offset;
}

void bar_item_init(struct bar_item* bar_item, struct bar_item* default_item) {
  bar_item->needs_update = true;
  bar_item->lazy = false;
  bar_item->drawing = true;
  bar_item->updates = true;
  bar_item->nospace = false;
  bar_item->selected = false;
  bar_item->counter = 0;
  bar_item->name = "";
  bar_item->type = BAR_ITEM;
  bar_item->update_frequency = 0;
  bar_item->cache_scripts = false;
  bar_item->script = "";
  bar_item->click_script = "";
  bar_item->position = BAR_POSITION_RIGHT;
  bar_item->associated_display = 0;
  bar_item->associated_space = 0;
  bar_item->associated_bar = 0;
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
  bar_item->draws_background = false;
  bar_item->background_color = rgba_color_from_hex(0x44ff0000);
  bar_item->background_border_color = rgba_color_from_hex(0x44ff0000);
  bar_item->background_height = 0;
  bar_item->background_corner_radius = 0;
  bar_item->background_border_width = 0;
  bar_item->background_padding_left = 0;
  bar_item->background_padding_right = 0;
  bar_item->y_offset = 0;
  bar_item->bounding_rects = NULL;
  bar_item->has_alias = false;

  if (default_item) bar_item_inherit_from_item(bar_item, default_item);

  bar_item_set_icon(bar_item, string_copy(""), false);
  bar_item_set_icon_font(bar_item, string_copy(bar_item->icon_font_name), true);
  bar_item_set_label_font(bar_item, string_copy(bar_item->label_font_name), true);
  bar_item_set_label(bar_item, string_copy(""), false);

  strncpy(&bar_item->signal_args.name[0][0], "NAME", 255);
  strncpy(&bar_item->signal_args.name[1][0], "SELECTED", 255);
  strncpy(&bar_item->signal_args.value[1][0], "false", 255);
}

void bar_item_append_associated_space(struct bar_item* bar_item, uint32_t bit) {
  bar_item->associated_space |= bit;
  if (bar_item->type == BAR_COMPONENT_SPACE) {
    bar_item->associated_space = bit;
    char sid_str[32];
    sprintf(sid_str, "%u", get_set_bit_position(bit));
    strncpy(&bar_item->signal_args.value[2][0], sid_str, 255);
  }
}

void bar_item_append_associated_display(struct bar_item* bar_item, uint32_t bit) {
  bar_item->associated_display |= bit;
  if (bar_item->type == BAR_COMPONENT_SPACE) {
    bar_item->associated_display = bit;
    char did_str[32];
    sprintf(did_str, "%u", get_set_bit_position(bit));
    strncpy(&bar_item->signal_args.value[3][0], did_str, 255);
  }
}

bool bar_item_is_shown(struct bar_item* bar_item) {
  if (bar_item->associated_bar & UINT32_MAX) return true;
  else return false;
}

void bar_item_append_associated_bar(struct bar_item* bar_item, uint32_t bit) {
  bar_item->associated_bar |= bit;
}

void bar_item_remove_associated_bar(struct bar_item* bar_item, uint32_t bit) {
  bar_item->associated_bar &= ~bit; 
}

void bar_item_reset_associated_bar(struct bar_item* bar_item) {
  bar_item->associated_bar = 0;
}

bool bar_item_update(struct bar_item* bar_item, bool forced) {
  if (!bar_item->updates || (bar_item->update_frequency == 0 && !forced)) return false;
  bar_item->counter++;
  if (bar_item->update_frequency <= bar_item->counter || forced) {
    bar_item->counter = 0;

    // Script Update
    if (strlen(bar_item->script) > 0) {
      fork_exec(bar_item->script, &bar_item->signal_args);
    }

    // Alias Update
    if (bar_item->has_alias && bar_item_is_shown(bar_item)) {
      alias_update_image(&bar_item->alias);
      bar_item_needs_update(bar_item);
      return true;
    }
  }
  return false;
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

void bar_item_set_type(struct bar_item* bar_item, char type) {
  bar_item->type = type;
  if (type == BAR_COMPONENT_SPACE) {
    bar_item->updates = false;
    strncpy(&bar_item->signal_args.name[2][0], "SID", 255);
    strncpy(&bar_item->signal_args.value[2][0], "0", 255);
    strncpy(&bar_item->signal_args.name[3][0], "DID", 255);
    strncpy(&bar_item->signal_args.value[3][0], "0", 255);
  }
  else if (type == BAR_COMPONENT_ALIAS) {
    bar_item->update_frequency = 1;
    bar_item->has_alias = true;
  }
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

void bar_item_set_drawing(struct bar_item* bar_item, bool state) {
  if (bar_item->drawing == state) return;
  bar_item->drawing = state;
  bar_item_needs_update(bar_item);
}

void bar_item_on_click(struct bar_item* bar_item) {
  if (!bar_item) return;
  if (bar_item && strlen(bar_item->click_script) > 0)
    fork_exec(bar_item->click_script, &bar_item->signal_args);
}

void bar_item_set_background_color(struct bar_item* bar_item, uint32_t color) {
  struct rgba_color target_color = rgba_color_from_hex(color);
  if (bar_item->background_color.r == target_color.r 
      && bar_item->background_color.g == target_color.g 
      && bar_item->background_color.b == target_color.b 
      && bar_item->background_color.a == target_color.a) return;
  bar_item->background_color = target_color;
  bar_item_set_draws_background(bar_item, true);
  bar_item_needs_update(bar_item);
}

void bar_item_set_background_border_color(struct bar_item* bar_item, uint32_t color) {
  struct rgba_color target_color = rgba_color_from_hex(color);
  if (bar_item->background_border_color.r == target_color.r 
      && bar_item->background_border_color.g == target_color.g 
      && bar_item->background_border_color.b == target_color.b 
      && bar_item->background_border_color.a == target_color.a) return;
  bar_item->background_border_color = target_color;
  bar_item_needs_update(bar_item);
}

void bar_item_set_draws_background(struct bar_item* bar_item, bool enabled) {
  if (bar_item->draws_background == enabled) return;
  bar_item->draws_background = enabled;
  bar_item_needs_update(bar_item);
}

void bar_item_set_background_height(struct bar_item* bar_item, uint32_t height) {
  if (bar_item->background_height == height) return;
  bar_item->background_height = height;
  bar_item_needs_update(bar_item);
}

void bar_item_set_background_border_width(struct bar_item* bar_item, uint32_t border_width) {
  if (bar_item->background_border_width == border_width) return;
  bar_item->background_border_width = border_width;
  bar_item_needs_update(bar_item);
}

void bar_item_set_background_corner_radius(struct bar_item* bar_item, uint32_t corner_radius) {
  if (bar_item->background_corner_radius == corner_radius) return;
  bar_item->background_corner_radius = corner_radius;
  bar_item_needs_update(bar_item);
}

void bar_item_set_yoffset(struct bar_item* bar_item, int offset) {
  if (bar_item->y_offset == offset) return;
  bar_item->y_offset = offset;
  bar_item_needs_update(bar_item);
}

CGRect bar_item_construct_bounding_rect(struct bar_item* bar_item) {
  CGRect bounding_rect;
  bounding_rect.origin = bar_item->icon_line.bounds.origin;
  bounding_rect.origin.x -= bar_item->icon_spacing_left;
  bounding_rect.origin.y = bar_item->icon_line.bounds.origin.y < bar_item->label_line.bounds.origin.y ? bar_item->icon_line.bounds.origin.y : bar_item->label_line.bounds.origin.y;
  bounding_rect.size.width = bar_item->label_line.bounds.size.width + bar_item->icon_line.bounds.size.width
                             + bar_item->icon_spacing_left + bar_item->icon_spacing_right
                             + bar_item->label_spacing_right + bar_item->label_spacing_left;
  bounding_rect.size.height = bar_item->label_line.bounds.size.height > bar_item->icon_line.bounds.size.height ? bar_item->label_line.bounds.size.height : bar_item->icon_line.bounds.size.height;
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
