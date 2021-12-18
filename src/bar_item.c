#include "bar_item.h"
#include "alias.h"
#include "custom_events.h"
#include "graph.h"
#include "group.h"
#include "misc/env_vars.h"
#include "misc/helpers.h"
#include "popup.h"
#include <stdint.h>
#include <string.h>

struct bar_item* bar_item_create() {
  struct bar_item* bar_item = malloc(sizeof(struct bar_item));
  memset(bar_item, 0, sizeof(struct bar_item));
  return bar_item;
}

void bar_item_clear_pointers(struct bar_item* bar_item) {
  bar_item->name = NULL;
  bar_item->script = NULL;
  bar_item->click_script = NULL;
  bar_item->bounding_rects = NULL;
  bar_item->group = NULL;
  bar_item->num_rects = 0;
  bar_item->signal_args.env_vars.vars = NULL;
  bar_item->signal_args.env_vars.count = 0;
  text_clear_pointers(&bar_item->icon);
  text_clear_pointers(&bar_item->label);
}

void bar_item_inherit_from_item(struct bar_item* bar_item, struct bar_item* ancestor) {
  text_destroy(&bar_item->icon);
  text_destroy(&bar_item->label);
  
  char* name = bar_item->name;
  char* script = bar_item->script;
  char* click_script = bar_item->click_script;

  memcpy(bar_item, ancestor, sizeof(struct bar_item));

  bar_item_clear_pointers(bar_item);
  bar_item->name = name;
  bar_item->script = script;
  bar_item->click_script = click_script;

  text_set_font(&bar_item->icon, string_copy(ancestor->icon.font_name), true);
  text_set_font(&bar_item->label, string_copy(ancestor->label.font_name), true);
  text_set_string(&bar_item->icon, string_copy(ancestor->icon.string), true);
  text_set_string(&bar_item->label, string_copy(ancestor->label.string), true);

  bar_item_set_script(bar_item, string_copy(ancestor->script));
  bar_item_set_click_script(bar_item, string_copy(ancestor->click_script));

  if (bar_item->type == BAR_COMPONENT_SPACE) {
    env_vars_set(&bar_item->signal_args.env_vars, string_copy("SID"), string_copy(env_vars_get_value_for_key(&ancestor->signal_args.env_vars, "DID")));
    env_vars_set(&bar_item->signal_args.env_vars, string_copy("DID"), string_copy(env_vars_get_value_for_key(&ancestor->signal_args.env_vars, "DID")));
  }
}

void bar_item_init(struct bar_item* bar_item, struct bar_item* default_item) {
  bar_item->needs_update = true;
  bar_item->lazy = false;
  bar_item->drawing = true;
  bar_item->updates = true;
  bar_item->updates_only_when_shown = false;
  bar_item->selected = false;
  bar_item->mouse_over = false;
  bar_item->counter = 0;
  bar_item->type = BAR_ITEM;
  bar_item->update_frequency = 0;
  bar_item->cache_scripts = false;
  bar_item->position = POSITION_RIGHT;
  bar_item->associated_display = 0;
  bar_item->associated_space = 0;
  bar_item->associated_bar = 0;

  bar_item->has_const_width = false;
  bar_item->custom_width = 0;

  bar_item->y_offset = 0;
  bar_item->num_rects = 0;
  bar_item->bounding_rects = NULL;
  bar_item->group = NULL;

  
  bar_item->has_alias = false;
  bar_item->has_graph = false;

  bar_item->name = string_copy("");
  bar_item->script = string_copy("");
  bar_item->click_script = string_copy("");

  text_init(&bar_item->icon);
  text_init(&bar_item->label);
  background_init(&bar_item->background);
  env_vars_init(&bar_item->signal_args.env_vars);
  popup_init(&bar_item->popup);
  
  if (default_item) bar_item_inherit_from_item(bar_item, default_item);

  env_vars_set(&bar_item->signal_args.env_vars, string_copy("SELECTED"), string_copy("false"));
}

void bar_item_append_associated_space(struct bar_item* bar_item, uint32_t bit) {
  if (bar_item->associated_space & bit) return;
  bar_item->associated_space |= bit;
  if (bar_item->type == BAR_COMPONENT_SPACE) {
    bar_item->associated_space = bit;
    char sid_str[32];
    sprintf(sid_str, "%u", get_set_bit_position(bit));
    env_vars_set(&bar_item->signal_args.env_vars, string_copy("SID"), string_copy(sid_str));
  }
}

void bar_item_append_associated_display(struct bar_item* bar_item, uint32_t bit) {
  if (bar_item->associated_display & bit) return;
  bar_item->associated_display |= bit;
  if (bar_item->type == BAR_COMPONENT_SPACE) {
    bar_item->associated_display = bit;
    char did_str[32];
    sprintf(did_str, "%u", get_set_bit_position(bit));
    env_vars_set(&bar_item->signal_args.env_vars, string_copy("DID"), string_copy(did_str));
  }
}

bool bar_item_is_shown(struct bar_item* bar_item) {
  if (bar_item->associated_bar & UINT32_MAX) return true;
  else return false;
}

void bar_item_append_associated_bar(struct bar_item* bar_item, uint32_t adid) {
  bar_item->associated_bar |= (1 << (adid - 1));
}

void bar_item_remove_associated_bar(struct bar_item* bar_item, uint32_t adid) {
  bar_item->associated_bar &= ~(1 << (adid - 1)); 
  bar_item_remove_bounding_rect_for_display(bar_item, adid);
}

void bar_item_reset_associated_bar(struct bar_item* bar_item) {
  bar_item->associated_bar = 0;
  for (uint32_t adid = 1; adid <= bar_item->num_rects; adid++)
    bar_item_remove_bounding_rect_for_display(bar_item, adid);
}

bool bar_item_update(struct bar_item* bar_item, char* sender, bool forced, struct env_vars* env_vars) {
  if ((!bar_item->updates || (bar_item->update_frequency == 0 && !sender)) && !forced) return false;
  bar_item->counter++;

  bool scheduled_update_needed = bar_item->update_frequency <= bar_item->counter;
  bool should_update = bar_item->updates_only_when_shown ? bar_item_is_shown(bar_item) : true;

  if (((scheduled_update_needed || sender) && should_update) || forced) {
    bar_item->counter = 0;

    // Script Update
    if (strlen(bar_item->script) > 0) {
      if (!env_vars) env_vars = &bar_item->signal_args.env_vars;
      else env_vars_set(env_vars, string_copy("NAME"), string_copy(bar_item->name));

      if (sender) env_vars_set(env_vars, string_copy("SENDER"), string_copy(sender));
      else env_vars_set(env_vars, string_copy("SENDER"), string_copy(forced ? "forced" : "routine"));
      fork_exec(bar_item->script, env_vars);
    }

    // Alias Update
    if (bar_item->has_alias && alias_update_image(&bar_item->alias)) {
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
  env_vars_set(&bar_item->signal_args.env_vars, string_copy("NAME"), string_copy(name));
}

void bar_item_set_type(struct bar_item* bar_item, char type) {
  bar_item->type = type;

  if (type == BAR_COMPONENT_SPACE) {
    if (strlen(bar_item->script) == 0) { 
        bar_item_set_script(bar_item, string_copy("sketchybar -m --set $NAME icon.highlight=$SELECTED"));
    }

    bar_item->update_mask |= UPDATE_SPACE_CHANGE;
    bar_item->updates = false;
    env_vars_set(&bar_item->signal_args.env_vars, string_copy("SID"), string_copy("0"));
    env_vars_set(&bar_item->signal_args.env_vars, string_copy("DID"), string_copy("0"));
  }
  else if (type == BAR_COMPONENT_ALIAS) {
    bar_item->update_frequency = 1;
    bar_item->has_alias = true;
    bar_item->updates_only_when_shown = true;
  }
  else if (type == BAR_COMPONENT_GRAPH) {
    bar_item->has_graph = true;
  }
  else if (type == BAR_COMPONENT_GROUP) {
    bar_item->drawing = false;
    bar_item->group = group_create();
    group_init(bar_item->group);
    group_add_member(bar_item->group, bar_item);
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

void bar_item_set_drawing(struct bar_item* bar_item, bool state) {
  if (bar_item->drawing == state) return;
  bar_item->drawing = state;
  bar_item_needs_update(bar_item);
}

void bar_item_on_click(struct bar_item* bar_item, uint32_t type, uint32_t modifier) {
  if (!bar_item) return;

  env_vars_set(&bar_item->signal_args.env_vars, string_copy("BUTTON"), string_copy(get_type_description(type)));
  env_vars_set(&bar_item->signal_args.env_vars, string_copy("MODIFIER"), string_copy(get_modifier_description(modifier)));

  if (strlen(bar_item->click_script) > 0)
    fork_exec(bar_item->click_script, &bar_item->signal_args.env_vars);
  if (bar_item->update_mask & UPDATE_MOUSE_CLICKED)
    bar_item_update(bar_item, COMMAND_SUBSCRIBE_MOUSE_CLICKED, true, NULL);
}

void bar_item_mouse_entered(struct bar_item* bar_item) {
  bar_item->mouse_over = true;
  if (bar_item->update_mask & UPDATE_MOUSE_ENTERED)
    bar_item_update(bar_item, COMMAND_SUBSCRIBE_MOUSE_ENTERED, true, NULL);
}

void bar_item_mouse_exited(struct bar_item* bar_item) {
  if (bar_item->mouse_over && (bar_item->update_mask & UPDATE_MOUSE_EXITED)) bar_item_update(bar_item, COMMAND_SUBSCRIBE_MOUSE_EXITED, true, NULL); 
  bar_item->mouse_over = false;
}


void bar_item_set_yoffset(struct bar_item* bar_item, int offset) {
  if (bar_item->y_offset == offset) return;
  bar_item->y_offset = offset;
  bar_item_needs_update(bar_item);
}

uint32_t bar_item_get_length(struct bar_item* bar_item, bool ignore_override) {
  if (bar_item->has_const_width && !ignore_override) return bar_item->custom_width - bar_item->background.padding_left - bar_item->background.padding_right;

  return text_get_length(&bar_item->icon, false)
         + text_get_length(&bar_item->label, false)
         + (bar_item->has_graph ? graph_get_length(&bar_item->graph) : 0)
         + (bar_item->has_alias ? alias_get_length(&bar_item->alias) : 0);
}

uint32_t bar_item_get_height(struct bar_item* bar_item) {
  uint32_t label_height = text_get_height(&bar_item->label);
  uint32_t icon_height = text_get_height(&bar_item->icon);
  uint32_t alias_height = alias_get_height(&bar_item->alias);
  
  uint32_t text_height = label_height > icon_height ? label_height : icon_height;
  return text_height > alias_height ? text_height : alias_height;
}

void bar_item_remove_bounding_rect_for_display(struct bar_item* bar_item, uint32_t adid) {
  if (bar_item->num_rects >= adid && bar_item->bounding_rects[adid - 1]) {
    free(bar_item->bounding_rects[adid - 1]);
    bar_item->bounding_rects[adid - 1] = NULL;
  }
}

CGRect bar_item_construct_bounding_rect(struct bar_item* bar_item) {
  CGRect bounding_rect;
  bounding_rect.origin = bar_item->icon.bounds.origin;
  bounding_rect.origin.y = bar_item->icon.bounds.origin.y < bar_item->label.bounds.origin.y ? bar_item->icon.bounds.origin.y : bar_item->label.bounds.origin.y + bar_item->y_offset;
  bounding_rect.size.width = bar_item_get_length(bar_item, false);
  bounding_rect.size.height = bar_item_get_height(bar_item);
  return bounding_rect;
}

void bar_item_set_bounding_rect_for_display(struct bar_item* bar_item, uint32_t adid, CGPoint bar_origin, uint32_t height) {
  if (bar_item->num_rects < adid) {
    bar_item->bounding_rects = (CGRect**) realloc(bar_item->bounding_rects, sizeof(CGRect*) * adid);
    memset(bar_item->bounding_rects + bar_item->num_rects, 0, sizeof(CGRect*) * (adid - bar_item->num_rects));
    bar_item->num_rects = adid;
  }
  if (!bar_item->bounding_rects[adid - 1]) {
    bar_item->bounding_rects[adid - 1] = malloc(sizeof(CGRect));
    memset(bar_item->bounding_rects[adid - 1], 0, sizeof(CGRect));
  }
  CGRect rect = bar_item_construct_bounding_rect(bar_item);
  bar_item->bounding_rects[adid - 1]->origin.x = rect.origin.x + bar_origin.x;
  bar_item->bounding_rects[adid - 1]->origin.y = -rect.origin.y - rect.size.height + bar_origin.y + height;
  bar_item->bounding_rects[adid - 1]->size = rect.size;
}

uint32_t bar_item_calculate_bounds(struct bar_item* bar_item, uint32_t bar_height, uint32_t x, uint32_t y) {
  uint32_t bar_item_length = bar_item_get_length(bar_item, false);
  uint32_t icon_position = x + bar_item->background.padding_left;
  uint32_t label_position = icon_position + text_get_length(&bar_item->icon, false);
  uint32_t sandwich_position = label_position;
  if (bar_item->has_graph) {
    label_position += graph_get_length(&bar_item->graph);
  } else if (bar_item->has_alias) {
    label_position += alias_get_length(&bar_item->alias); 
  }

  if (bar_item->group && group_is_first_member(bar_item->group, bar_item))
    group_calculate_bounds(bar_item->group, (bar_item->position == POSITION_RIGHT || bar_item->position == POSITION_CENTER_LEFT) ? 
        (icon_position - group_get_length(bar_item->group) + bar_item_length) : 
        icon_position, y, bar_item->position == POSITION_RIGHT || bar_item->position == POSITION_CENTER_LEFT);

  text_calculate_bounds(&bar_item->icon, icon_position, y + bar_item->y_offset);
  text_calculate_bounds(&bar_item->label, label_position, y + bar_item->y_offset);

  if (bar_item->has_alias)
    alias_calculate_bounds(&bar_item->alias, sandwich_position, y + bar_item->y_offset);

  if (bar_item->has_graph) {
    bar_item->graph.bounds.size.height = bar_item->background.enabled ? (bar_item->background.bounds.size.height - bar_item->background.border_width - 1)
                                                                    : (bar_height - (g_bar_manager.background.border_width + 1));
    graph_calculate_bounds(&bar_item->graph, sandwich_position, y + bar_item->y_offset);
  }

  bar_item->background.bounds.size.height = bar_item->background.overrides_height ? bar_item->background.bounds.size.height
                                                                                  : (bar_height - (g_bar_manager.background.border_width + 1));
  bar_item->background.bounds.size.width = bar_item_length;
  background_calculate_bounds(&bar_item->background, icon_position, y + bar_item->y_offset);
  return bar_item_length;
}

void bar_item_draw(struct bar_item* bar_item, CGContextRef context) {
  if (bar_item->group && group_is_first_member(bar_item->group, bar_item))
    group_draw(bar_item->group, context);

  background_draw(&bar_item->background, context);
  text_draw(&bar_item->icon, context);
  text_draw(&bar_item->label, context);

  if (bar_item->has_alias)
    alias_draw(&bar_item->alias, context);
  if (bar_item->has_graph)
    graph_draw(&bar_item->graph, context);
  if (bar_item->popup.drawing)
    popup_draw(&bar_item->popup);
}

void bar_item_destroy(struct bar_item* bar_item) {
  if (bar_item->name) free(bar_item->name);
  if (bar_item->script && !bar_item->cache_scripts) free(bar_item->script);
  if (bar_item->click_script && !bar_item->cache_scripts) free(bar_item->click_script);

  text_destroy(&bar_item->icon);
  text_destroy(&bar_item->label);

  if (bar_item->bounding_rects) {  
    for (int j = 0; j < bar_item->num_rects; j++) {
      free(bar_item->bounding_rects[j]);
    }
    free(bar_item->bounding_rects);
  }
  if (bar_item->has_graph) {
    graph_destroy(&bar_item->graph);
  }
  if (bar_item->group && bar_item->type == BAR_COMPONENT_GROUP) group_destroy(bar_item->group);
  else if (bar_item->group) group_remove_member(bar_item->group, bar_item);
  env_vars_destroy(&bar_item->signal_args.env_vars);
  free(bar_item);
}

void bar_item_serialize(struct bar_item* bar_item, FILE* rsp) {
  fprintf(rsp, "{\n"
               "\t\"name\": \"%s\",\n"
               "\t\"type\": \"%c\",\n"
               "\t\"text\": {\n"
               "\t\t\"icon\": \"%s\",\n"
               "\t\t\"label\": \"%s\",\n"
               "\t\t\"icon.font\": \"%s\",\n"
               "\t\t\"label.font\": \"%s\"\n"
               "\t},\n"
               "\t\"geometry\": {\n"
               "\t\t\"position\": \"%c\",\n"
               "\t\t\"fixed_width\": %d,\n"
               "\t\t\"background.padding_left\": %d,\n"
               "\t\t\"background.padding_right\": %d,\n"
               "\t\t\"icon.padding_left\": %d,\n"
               "\t\t\"icon.padding_right\": %d,\n"
               "\t\t\"label.padding_left\": %d,\n"
               "\t\t\"label.padding_right\": %d\n"
               "\t},\n"
               "\t\"style\": {\n"
               "\t\t\"icon.color\": \"0x%x\",\n"
               "\t\t\"icon.highlight_color:\": \"0x%x\",\n"
               "\t\t\"label.color\": \"0x%x\",\n"
               "\t\t\"label.highlight_color:\": \"0x%x\",\n"
               "\t\t\"background.drawing\": %d,\n"
               "\t\t\"background.height\": %u,\n"
               "\t\t\"background.corner_radius\": %u,\n"
               "\t\t\"background.border_width\": %u,\n"
               "\t\t\"background.color\": \"0x%x\",\n"
               "\t\t\"background.border_color\": \"0x%x\"\n"
               "\t},\n"
               "\t\"state\": {\n"
               "\t\t\"drawing\": %d,\n"
               "\t\t\"updates\": %d,\n"
               "\t\t\"lazy\": %d,\n"
               "\t\t\"cache_scripts\": %d,\n"
               "\t\t\"associated_bar_mask\": %u,\n"
               "\t\t\"associated_display_mask\": %u,\n"
               "\t\t\"associated_space_mask\": %u,\n"
               "\t\t\"update_mask\": %llu\n"
               "\t},\n"
               "\t\"bounding_rects\": {\n",
               bar_item->name,
               bar_item->type,
               bar_item->icon.string,
               bar_item->label.string,
               bar_item->icon.font_name,
               bar_item->label.font_name,
               bar_item->position,
               bar_item->has_const_width ? bar_item->custom_width : -1,
               bar_item->background.padding_left,
               bar_item->background.padding_right,
               bar_item->icon.padding_left,
               bar_item->icon.padding_right,
               bar_item->label.padding_left,
               bar_item->label.padding_right,
               hex_from_rgba_color(bar_item->icon.color),
               hex_from_rgba_color(bar_item->icon.highlight_color),
               hex_from_rgba_color(bar_item->label.color),
               hex_from_rgba_color(bar_item->label.highlight_color),
               bar_item->background.enabled,
               (uint32_t)bar_item->background.bounds.size.height,
               bar_item->background.corner_radius,
               bar_item->background.border_width,
               hex_from_rgba_color(bar_item->background.color),
               hex_from_rgba_color(bar_item->background.border_color),
               bar_item->drawing,
               bar_item->updates,
               bar_item->lazy,
               bar_item->cache_scripts,
               bar_item->associated_bar,
               bar_item->associated_display,
               bar_item->associated_space,
               bar_item->update_mask);

  int counter = 0;
  for (int i = 0; i < bar_item->num_rects; i++) {
    if (!bar_item->bounding_rects[i]) continue;
    if (counter++ > 0) fprintf(rsp, ",\n");
    fprintf(rsp, "\t\t\"display-%d\": {\n"
            "\t\t\t\"origin\": [ %f, %f ],\n"
            "\t\t\t\"size\": [ %f, %f ]\n\t\t}",
            i + 1,
            bar_item->bounding_rects[i]->origin.x,
            bar_item->bounding_rects[i]->origin.y,
            bar_item->bounding_rects[i]->size.width,
            bar_item->bounding_rects[i]->size.height);
  } 
  fprintf(rsp, "\n\t}");

  if (bar_item->type == BAR_COMPONENT_GROUP && bar_item->group) {
    group_serialize(bar_item->group, rsp);
  } else if (bar_item->type == BAR_COMPONENT_GRAPH) {
    graph_serialize(&bar_item->graph, rsp);
  }
  fprintf(rsp, "\n}\n");
}
