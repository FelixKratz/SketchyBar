#include "bar_item.h"
#include "bar_manager.h"
#include "event.h"
#include "volume.h"
#include "power.h"
#include "media.h"
#include "app_windows.h"

struct bar_item* bar_item_create() {
  struct bar_item* bar_item = malloc(sizeof(struct bar_item));
  memset(bar_item, 0, sizeof(struct bar_item));
  return bar_item;
}

void bar_item_init(struct bar_item* bar_item, struct bar_item* default_item) {
  bar_item->needs_update = true;
  bar_item->lazy = true;
  bar_item->drawing = true;
  bar_item->updates = true;
  bar_item->updates_only_when_shown = false;
  bar_item->selected = false;
  bar_item->ignore_association = false;
  bar_item->overrides_association = false;
  bar_item->counter = 0;
  bar_item->type = BAR_ITEM;
  bar_item->update_frequency = 0;
  bar_item->position = POSITION_LEFT;
  bar_item->align = POSITION_LEFT;
  bar_item->associated_to_active_display = false;
  bar_item->associated_display = 0;
  bar_item->associated_space = 0;
  bar_item->associated_bar = 0;
  bar_item->blur_radius = 0;
  bar_item->event_port = 0;
  bar_item->shadow = false;
  bar_item->scroll_texts = false;
  bar_item->mouse_over = false;

  bar_item->has_const_width = false;
  bar_item->custom_width = 0;

  bar_item->y_offset = 0;
  
  bar_item->has_alias = false;
  bar_item->has_graph = false;

  bar_item->name = NULL;
  bar_item->script = NULL;
  bar_item->click_script = NULL;

  bar_item->group = NULL;
  bar_item->parent = NULL;

  text_init(&bar_item->icon);
  text_init(&bar_item->label);
  background_init(&bar_item->background);
  env_vars_init(&bar_item->signal_args.env_vars);
  popup_init(&bar_item->popup, bar_item);
  graph_init(&bar_item->graph);
  alias_init(&bar_item->alias);
  slider_init(&bar_item->slider);
  
  if (default_item) bar_item_inherit_from_item(bar_item, default_item);
}

void bar_item_append_associated_space(struct bar_item* bar_item, uint32_t bit) {
  if (bar_item->associated_space & bit) return;
  bar_item->associated_space |= bit;
  if (bar_item->type == BAR_COMPONENT_SPACE) {
    bar_item->associated_space = bit;
    char sid_str[32];
    snprintf(sid_str, 32, "%u", get_set_bit_position(bit));
    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("SID"),
                 string_copy(sid_str)            );
  }
}

void bar_item_append_associated_display(struct bar_item* bar_item, uint32_t bit) {
  if (bar_item->associated_display & bit) return;
  bar_item->associated_display |= bit;
  if (bar_item->type == BAR_COMPONENT_SPACE) {
    bar_item->overrides_association = true;
    bar_item->associated_display = bit;
    char did_str[32];
    snprintf(did_str, 32, "%u", get_set_bit_position(bit));
    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("DID"),
                 string_copy(did_str)            );
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
}

void bar_item_reset_associated_bar(struct bar_item* bar_item) {
  bar_item->associated_bar = 0;
}

bool bar_item_update(struct bar_item* bar_item, char* sender, bool forced, struct env_vars* env_vars) {
  bool is_shown = bar_item_is_shown(bar_item);
  if (is_shown && bar_item->scroll_texts && (bar_item->counter % 15 == 0)) {
    text_animate_scroll(&bar_item->icon);
    text_animate_scroll(&bar_item->label);
    if (bar_item->type == BAR_COMPONENT_SLIDER)
      text_animate_scroll(&bar_item->slider.knob);
  }

  bar_item->counter++;

  if ((!bar_item->updates || (bar_item->update_frequency == 0 && !sender))
      && !forced                                                          ) {
    return false;
  }

  bool scheduled_update_needed = bar_item->update_frequency
                                  <= bar_item->counter;

  bool should_update = bar_item->updates_only_when_shown
                       ? is_shown
                       : true;

  if (((scheduled_update_needed || sender) && should_update) || forced) {
    bar_item->counter = 0;

    if ((bar_item->script && strlen(bar_item->script) > 0)
         || bar_item->event_port                          ) {
      if (!env_vars)
        env_vars = &bar_item->signal_args.env_vars;
      else {
        for (int i = 0; i < bar_item->signal_args.env_vars.count; i++) {
          env_vars_set(env_vars,
                       string_copy(bar_item->signal_args.env_vars.vars[i]->key),
                       string_copy(bar_item->signal_args.env_vars.vars[i]->value));
        }
        env_vars_set(env_vars,
                     string_copy("NAME"),
                     string_copy(bar_item->name));
      }

      if (sender)
        env_vars_set(env_vars, string_copy("SENDER"), string_copy(sender));
      else
        env_vars_set(env_vars,
                     string_copy("SENDER"),
                     string_copy(forced ? "forced" : "routine"));
    }
    // Script Update
    if (bar_item->script && strlen(bar_item->script) > 0) {
      fork_exec(bar_item->script, env_vars);
    }

    // Mach events
    if (bar_item->event_port) {
      uint32_t len = 0;
      char* message = env_vars_copy_serialized_representation(env_vars, &len);

      mach_send_message(bar_item->event_port, message, len, false);
      free(message);
    }
  }

  return false;
}

void bar_item_needs_update(struct bar_item* bar_item) {
  bar_item->needs_update = true;
}

void bar_item_cancel_drag(struct bar_item* bar_item) {
  if (bar_item->has_slider) {
    char perc_str[8];
    snprintf(perc_str, 8, "%d", bar_item->slider.percentage);
    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("PERCENTAGE"),
                 string_copy(perc_str)           );

    slider_cancel_drag(&bar_item->slider);
  }
}

void bar_item_on_drag(struct bar_item* bar_item, CGPoint point) {
  if (bar_item->has_slider) {
    if (slider_handle_drag(&bar_item->slider, point)) {
      bar_item_needs_update(bar_item);
    }
  }
}

void bar_item_on_click(struct bar_item* bar_item, uint32_t type, uint32_t mouse_button_code, uint32_t modifier, CGPoint point) {
  if (!bar_item) return;

  struct env_vars env_vars;
  env_vars_init(&env_vars);
  char info_str[256];
  snprintf(info_str, 256, "{\n"
                         "\t\"button\": \"%s\",\n"
                         "\t\"button_code\": %u,\n"
                         "\t\"modifier\": \"%s\",\n"
                         "\t\"modfier_code\": %u\n"
                         "}\n",
                         get_type_description(type),
                         mouse_button_code,
                         get_modifier_description(modifier),
                         modifier                           );

  env_vars_set(&env_vars, string_copy("INFO"), string_copy(info_str));

  env_vars_set(&env_vars,
               string_copy("BUTTON"),
               string_copy(get_type_description(type)));

  env_vars_set(&env_vars,
               string_copy("MODIFIER"),
               string_copy(get_modifier_description(modifier)));

  if (bar_item->has_slider) {
    if (bar_item->slider.is_dragged
        || CGRectContainsPoint(bar_item->slider.background.bounds, point)) {
      if (slider_handle_drag(&bar_item->slider, point)) {
        bar_item_needs_update(bar_item);
      }

      bar_item_cancel_drag(bar_item);
    } else {
      env_vars_destroy(&env_vars);
      return;
    }
  }

  if (bar_item->click_script && strlen(bar_item->click_script) > 0) {
    for (int i = 0; i < bar_item->signal_args.env_vars.count; i++) {
      env_vars_set(&env_vars,
                   string_copy(bar_item->signal_args.env_vars.vars[i]->key),
                   string_copy(bar_item->signal_args.env_vars.vars[i]->value));
    }

    fork_exec(bar_item->click_script, &env_vars);
  }
  if (bar_item->update_mask & UPDATE_MOUSE_CLICKED)
    bar_item_update(bar_item,
                    COMMAND_SUBSCRIBE_MOUSE_CLICKED,
                    true,
                    &env_vars                       );

  env_vars_destroy(&env_vars);
}

void bar_item_on_scroll(struct bar_item* bar_item, int scroll_delta) {
  if (!bar_item) return;

  struct env_vars env_vars;
  env_vars_init(&env_vars);
  char info_str[256];
  snprintf(info_str, 256, "{\n"
                          "\t\"delta\": %d\n"
                          "}\n",
                          scroll_delta       );

  env_vars_set(&env_vars, string_copy("INFO"), string_copy(info_str));

  char delta_ver_str[32];
  snprintf(delta_ver_str, 32, "%d", scroll_delta);
  env_vars_set(&env_vars,
               string_copy("SCROLL_DELTA"),
               string_copy(delta_ver_str));

  if (bar_item->update_mask & UPDATE_MOUSE_SCROLLED)
    bar_item_update(bar_item,
                    COMMAND_SUBSCRIBE_MOUSE_SCROLLED,
                    true,
                    &env_vars                        );


  env_vars_destroy(&env_vars);
}

void bar_item_mouse_entered(struct bar_item* bar_item) {
  if (bar_item->update_mask & UPDATE_MOUSE_ENTERED && !bar_item->mouse_over) {
    bar_item_update(bar_item, COMMAND_SUBSCRIBE_MOUSE_ENTERED, true, NULL);
  }
  bar_item->mouse_over = true;
}

void bar_item_mouse_exited(struct bar_item* bar_item) {
  if (bar_item->update_mask & UPDATE_MOUSE_EXITED && bar_item->mouse_over) {
    bar_item_update(bar_item, COMMAND_SUBSCRIBE_MOUSE_EXITED, true, NULL); 
  }
  bar_item->mouse_over = false;
}

static bool bar_item_set_drawing(struct bar_item* bar_item, bool state) {
  if (bar_item->drawing == state) return false;
  bar_item->drawing = state;
  return true;
}

static void bar_item_set_script(struct bar_item* bar_item, char* script) {
  if (!script) return;

  if (bar_item->script && strcmp(bar_item->script, script) == 0) {
    free(script);
    return;
  }

  if (script != bar_item->script && bar_item->script)
    free(bar_item->script);

  char* path = resolve_path(script);
  if (path) bar_item->script = path;
}

static void bar_item_set_click_script(struct bar_item* bar_item, char* script) {
  if (!script) return;

  if (bar_item->click_script && strcmp(bar_item->click_script, script) == 0) {
    free(script);
    return;
  }

  if (script != bar_item->click_script && bar_item->click_script)
    free(bar_item->click_script);

  char* path = resolve_path(script);
  if (path) bar_item->click_script = path;
}

static bool bar_item_set_yoffset(struct bar_item* bar_item, int offset) {
  if (bar_item->y_offset == offset) return false;
  bar_item->y_offset = offset;
  return true;
}

static bool bar_item_set_blur_radius(struct bar_item* bar_item, uint32_t radius) {
  if (bar_item->blur_radius == radius) return false;
  bar_item->blur_radius = radius;
  for (int i = 0; i < bar_item->num_windows; i++) {
    if (!bar_item->windows[i]) continue;
    window_set_blur_radius(bar_item->windows[i], radius);
  }
  return true;
}

static bool bar_item_set_width(struct bar_item* bar_item, int width) {
  if (width < 0) {
    bool prev = bar_item->has_const_width;
    bar_item->has_const_width = false;
    return prev != bar_item->has_const_width;
  }
  if (bar_item->custom_width == width && bar_item->has_const_width)
    return false;

  bar_item->custom_width = width;
  bar_item->has_const_width = true;
  return true;
}

static void bar_item_set_event_port(struct bar_item* bar_item, char* bs_name) {
  mach_port_t port = mach_get_bs_port(bs_name);
  bar_item->event_port = port;
}

bool bar_item_set_name(struct bar_item* bar_item, char* name) {
  if (!name) return false;

  if (bar_item->name && strcmp(bar_item->name, name) == 0) {
    free(name);
    return true;
  }

  if (strlen(name) == 0) return false;

  if (name != bar_item->name && bar_item->name) free(bar_item->name);
  bar_item->name = name;
  env_vars_set(&bar_item->signal_args.env_vars,
               string_copy("NAME"),
               string_copy(name)               );
  return true;
}

bool bar_item_set_type(struct bar_item* bar_item, char* type) {
  bool success = true;

  if (string_equals(type, TYPE_SPACE)) {
    bar_item->type = BAR_COMPONENT_SPACE;
  } else if (string_equals(type, TYPE_ALIAS)) {
    bar_item->type = BAR_COMPONENT_ALIAS;
  } else if (string_equals(type, TYPE_GROUP)) {
    bar_item->type = BAR_COMPONENT_GROUP;
  } else if (string_equals(type, TYPE_GRAPH)) {
    bar_item->type = BAR_COMPONENT_GRAPH;
  } else if (string_equals(type, TYPE_SLIDER)) {
    bar_item->type = BAR_COMPONENT_SLIDER;
  } else {
    bar_item->type = BAR_ITEM;
    success = string_equals(type, TYPE_ITEM);
  }

  if (bar_item->type == BAR_COMPONENT_SPACE) {
    if (!bar_item->script) { 
        bar_item_set_script(bar_item,
            string_copy("sketchybar -m --set $NAME icon.highlight=$SELECTED"));
    }

    bar_item->update_mask |= UPDATE_SPACE_CHANGE;
    bar_item->updates = false;
    bar_item->updates_only_when_shown = false;
    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("SELECTED"),
                 string_copy("false")            );

    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("SID"),
                 string_copy("0")                );

    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("DID"),
                 string_copy("0")                );
  }
  else if (bar_item->type == BAR_COMPONENT_ALIAS) {
    bar_item->has_alias = true;
  }
  else if (bar_item->type == BAR_COMPONENT_GRAPH) {
    bar_item->has_graph = true;
  }
  else if (bar_item->type == BAR_COMPONENT_SLIDER) {
    bar_item->has_slider = true;
  }
  else if (bar_item->type == BAR_COMPONENT_GROUP) {
    bar_item->group = group_create();
    group_init(bar_item->group);
    group_add_member(bar_item->group, bar_item);
  }
  return success;
}

bool bar_item_set_position(struct bar_item* bar_item, char* position) {
  if (!position || strlen(position) == 0) return false;

  switch (position[0]) {
    case POSITION_LEFT:
      break;
    case POSITION_CENTER_LEFT:
      break;
    case POSITION_CENTER:
      break;
    case POSITION_CENTER_RIGHT:
      break;
    case POSITION_RIGHT:
      break;
    case POSITION_POPUP:
      break;
    default:
      return false;
  }

  bar_item->position = position[0];
  if (position[0] != POSITION_POPUP)
    bar_item->align = position[0];
  return true;
}

bool bar_item_set_media_cover(struct bar_item* bar_item, struct image* image) {
  bool linked = (bar_item->background.image.link == image
                 ||bar_item->icon.background.image.link == image
                 ||bar_item->label.background.image.link == image);
  if (linked) bar_item_needs_update(bar_item);
  return linked && bar_item_is_shown(bar_item);
}

static uint32_t bar_item_get_content_length(struct bar_item* bar_item) {
  int length = text_get_length(&bar_item->icon, false)
         + text_get_length(&bar_item->label, false)
         + (bar_item->has_graph  ? graph_get_length(&bar_item->graph) : 0)
         + (bar_item->has_slider ? slider_get_length(&bar_item->slider) : 0)
         + (bar_item->has_alias  ? alias_get_length(&bar_item->alias) : 0);
 
  return max(length, 0);
}

uint32_t bar_item_get_length(struct bar_item* bar_item, bool ignore_override) {
  uint32_t content_length = bar_item_get_content_length(bar_item);
  if (bar_item->background.enabled && bar_item->background.image.enabled) {
    CGSize image_size = image_get_size(&bar_item->background.image);

    if (image_size.width > content_length) content_length = image_size.width;
  }

  if (bar_item->has_const_width && (!ignore_override
                                    || bar_item->custom_width
                                       > content_length      )) {
    return bar_item->custom_width;
  }

  return content_length;
}

uint32_t bar_item_get_height(struct bar_item* bar_item) {
  uint32_t label_height = text_get_height(&bar_item->label);
  uint32_t icon_height = text_get_height(&bar_item->icon);
  uint32_t alias_height = alias_get_height(&bar_item->alias);
  
  uint32_t text_height = max(label_height, icon_height);
  uint32_t item_height = max(text_height, alias_height);

  uint32_t background_height = 0;
  if (bar_item->background.enabled) {
    uint32_t image_height = bar_item->background.image.enabled
                            ? image_get_size(&bar_item->background.image).height
                            : 0;

    background_height = max(image_height,
                            bar_item->background.bounds.size.height);
  }

  return max(item_height, background_height);
}

struct window* bar_item_get_window(struct bar_item* bar_item, uint32_t adid) {
  if (adid < 1 || !bar_item) return NULL;
  if (bar_item->num_windows < adid) {
    bar_item->windows = (struct window**) realloc(bar_item->windows,
                                                  sizeof(struct window*)*adid);
    memset(bar_item->windows + bar_item->num_windows,
           0,
           sizeof(struct window*) * (adid - bar_item->num_windows));

    bar_item->num_windows = adid;
  }

  if (!bar_item->windows[adid - 1]) {
    bar_item->windows[adid - 1] = malloc(sizeof(struct window));
    window_init(bar_item->windows[adid - 1]);
    window_create(bar_item->windows[adid - 1],
                  (CGRect){{g_nirvana.x,g_nirvana.y}, {1, 1}});
    if (!bar_item->shadow) window_disable_shadow(bar_item->windows[adid - 1]);
    window_set_blur_radius(bar_item->windows[adid - 1], bar_item->blur_radius);
    context_set_font_smoothing(bar_item->windows[adid - 1]->context,
                               g_bar_manager.font_smoothing         );
    
    if (bar_item->parent)
      bar_item->parent->popup.needs_ordering = true;
    else
      g_bar_manager.needs_ordering = true;
  }

  return bar_item->windows[adid - 1];
}

void bar_item_remove_window(struct bar_item* bar_item, uint32_t adid) {
  if (bar_item->num_windows >= adid && bar_item->windows[adid - 1]) {
    window_close(bar_item->windows[adid - 1]);
    free(bar_item->windows[adid - 1]);
    bar_item->windows[adid - 1] = NULL;
  }
}

CGPoint bar_item_calculate_shadow_offsets(struct bar_item* bar_item) {
  CGPoint offset; 
  offset.x = (int)((bar_item->background.shadow.enabled
                ? max(-bar_item->background.shadow.offset.x, 0)
                : 0)
             + (bar_item->icon.shadow.enabled
                ? max(-bar_item->icon.shadow.offset.x, 0)
                : 0)
             + (bar_item->icon.background.shadow.enabled
                ? max(-bar_item->icon.background.shadow.offset.x, 0)
                : 0)
             + (bar_item->label.background.shadow.enabled
                ? max(-bar_item->label.background.shadow.offset.x, 0)
                : 0)
             + (bar_item->label.shadow.enabled
                ? max(-bar_item->label.shadow.offset.x, 0)
                : 0));

  offset.y = (int)((bar_item->background.shadow.enabled
                 ? max(bar_item->background.shadow.offset.x,0)
                 : 0)
              + (bar_item->icon.shadow.enabled
                 ? max(bar_item->icon.shadow.offset.x, 0)
                 : 0)
              + (bar_item->icon.background.shadow.enabled
                 ? max(bar_item->icon.background.shadow.offset.x, 0)
                 : 0)
              + (bar_item->label.background.shadow.enabled
                 ? max(bar_item->label.background.shadow.offset.x, 0)
                 : 0)
              + (bar_item->label.shadow.enabled
                 ? max(bar_item->label.shadow.offset.x, 0)
                 : 0));
  return offset;
}

uint32_t bar_item_calculate_bounds(struct bar_item* bar_item, uint32_t bar_height, uint32_t x, uint32_t y) {
  uint32_t content_x = x;
  uint32_t content_y = y;

  uint32_t bar_item_length = bar_item_get_length(bar_item, false);
  uint32_t bar_item_content_length = bar_item_get_content_length(bar_item);
  if (bar_item_length > bar_item_content_length) {
    if (bar_item->align == POSITION_CENTER)
      content_x += (bar_item_length - bar_item_content_length) / 2;
    else if (bar_item->align == POSITION_RIGHT)
      content_x += bar_item_length - bar_item_content_length;
  }

  uint32_t icon_position = content_x;
  uint32_t label_position = icon_position + text_get_length(&bar_item->icon,
                                                            false           );

  uint32_t sandwich_position = label_position;
  if (bar_item->has_graph) {
    label_position += graph_get_length(&bar_item->graph);
  } else if (bar_item->has_alias) {
    label_position += alias_get_length(&bar_item->alias); 
  } else if (bar_item->has_slider) {
    label_position += slider_get_length(&bar_item->slider); 
  }


  text_calculate_bounds(&bar_item->icon,
                        icon_position,
                        content_y + bar_item->y_offset);

  text_calculate_bounds(&bar_item->label,
                        label_position,
                        content_y + bar_item->y_offset);

  if (bar_item->has_alias)
    alias_calculate_bounds(&bar_item->alias,
                           sandwich_position,
                           content_y + bar_item->y_offset);

  if (bar_item->has_slider)
    slider_calculate_bounds(&bar_item->slider,
                            sandwich_position,
                            content_y + bar_item->y_offset);

  if (bar_item->has_graph) {
    uint32_t height = bar_item->background.enabled
                      ? (bar_item->background.bounds.size.height
                         - bar_item->background.border_width - 1)
                      : (bar_height
                         - (g_bar_manager.background.border_width + 1));

    graph_calculate_bounds(&bar_item->graph,
                           sandwich_position,
                           content_y + bar_item->y_offset,
                           height                         );
  }

  if (bar_item->background.enabled) {
    uint32_t height = bar_item->background.overrides_height
                      ? bar_item->background.bounds.size.height
                      : (bar_height
                         - (g_bar_manager.background.border_width + 1));

    background_calculate_bounds(&bar_item->background,
                                x,
                                content_y + bar_item->y_offset,
                                bar_item_length,
                                height                         );
  }

  return bar_item_length;
}

bool bar_item_clip_needs_update_for_bar(struct bar_item* bar_item, struct bar* bar) {
  bool needs_update = false;

  needs_update |= background_clip_needs_update(&bar_item->background, bar)
               || background_clip_needs_update(&bar_item->icon.background, bar)
               || background_clip_needs_update(&bar_item->label.background, bar);

  return needs_update;
}

bool bar_item_clips_bar(struct bar_item* bar_item) {
  return background_clips_bar(&bar_item->background)
         || background_clips_bar(&bar_item->icon.background)
         || background_clips_bar(&bar_item->label.background);
}

void bar_item_clip_bar(struct bar_item* bar_item, int offset, struct bar* bar) {
  background_clip_bar(&bar_item->background, offset, bar);
  background_clip_bar(&bar_item->icon.background, offset, bar);
  background_clip_bar(&bar_item->label.background, offset, bar);
}

void bar_item_draw(struct bar_item* bar_item, CGContextRef context) {
  background_draw(&bar_item->background, context);
  if (bar_item->type == BAR_COMPONENT_GROUP) return;

  text_draw(&bar_item->icon, context);
  text_draw(&bar_item->label, context);

  if (bar_item->has_alias) alias_draw(&bar_item->alias, context);
  if (bar_item->has_graph) graph_draw(&bar_item->graph, context);
  if (bar_item->has_slider) slider_draw(&bar_item->slider, context);
}

void bar_item_change_space(struct bar_item* bar_item, uint64_t dsid, uint32_t adid) {
  if (bar_item->num_windows >= adid && bar_item->windows[adid - 1]) {
    window_send_to_space(bar_item->windows[adid - 1], dsid);
  }
  popup_change_space(&bar_item->popup, dsid, adid);
}

static void bar_item_clear_pointers(struct bar_item* bar_item) {
  bar_item->name = NULL;
  bar_item->script = NULL;
  bar_item->click_script = NULL;
  bar_item->group = NULL;
  bar_item->signal_args.env_vars.vars = NULL;
  bar_item->signal_args.env_vars.count = 0;
  bar_item->windows = NULL;
  bar_item->num_windows = 0;
  text_clear_pointers(&bar_item->icon);
  text_clear_pointers(&bar_item->label);
  background_clear_pointers(&bar_item->background);
  slider_clear_pointers(&bar_item->slider);
  popup_clear_pointers(&bar_item->popup);
  bar_item->popup.host = bar_item;
}

void bar_item_inherit_from_item(struct bar_item* bar_item, struct bar_item* ancestor) {
  text_destroy(&bar_item->icon);
  text_destroy(&bar_item->label);
  text_destroy(&bar_item->slider.knob);
  
  char* name = bar_item->name;
  char* script = bar_item->script;
  char* click_script = bar_item->click_script;

  memcpy(bar_item, ancestor, sizeof(struct bar_item));
  bar_item_clear_pointers(bar_item);

  bar_item->name = name;
  bar_item->script = script;
  bar_item->click_script = click_script;

  text_copy(&bar_item->icon, &ancestor->icon);
  text_copy(&bar_item->label, &ancestor->label);
  text_copy(&bar_item->slider.knob, &ancestor->slider.knob);

  if (ancestor->script)
    bar_item_set_script(bar_item, string_copy(ancestor->script));
  if (ancestor->click_script)
    bar_item_set_click_script(bar_item, string_copy(ancestor->click_script));

  image_copy(&bar_item->background.image,
             ancestor->background.image.image_ref);

  image_copy(&bar_item->icon.background.image,
             ancestor->icon.background.image.image_ref);

  image_copy(&bar_item->label.background.image,
             ancestor->label.background.image.image_ref);

  if (bar_item->type == BAR_COMPONENT_SPACE) {
    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("SELECTED"),
                 string_copy("false")            );

    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("SID"),
                 string_copy(env_vars_get_value_for_key(&ancestor->signal_args.env_vars,
                                                        "DID")                          ));
    env_vars_set(&bar_item->signal_args.env_vars,
                 string_copy("DID"),
                 string_copy(env_vars_get_value_for_key(&ancestor->signal_args.env_vars,
                                                        "DID")                          ));
  }
}

void bar_item_destroy(struct bar_item* bar_item, bool free_memory) {
  if (bar_item->name) free(bar_item->name);
  if (bar_item->script) free(bar_item->script);
  if (bar_item->click_script) free(bar_item->click_script);

  text_destroy(&bar_item->icon);
  text_destroy(&bar_item->label);

  graph_destroy(&bar_item->graph);
  alias_destroy(&bar_item->alias);
  slider_destroy(&bar_item->slider);

  if (bar_item->group && bar_item->type == BAR_COMPONENT_GROUP)
    group_destroy(bar_item->group);
  else if (bar_item->group)
    group_remove_member(bar_item->group, bar_item);

  env_vars_destroy(&bar_item->signal_args.env_vars);
  popup_destroy(&bar_item->popup);
  background_destroy(&bar_item->background);

  for (int j = 1; j <= bar_item->num_windows; j++) {
    bar_item_remove_window(bar_item, j);
  }
  if (bar_item->windows) free(bar_item->windows);

  if (free_memory) free(bar_item);
}

void bar_item_serialize(struct bar_item* bar_item, FILE* rsp) {
  char type[32] = { 0 };
  switch (bar_item->type) {
    case BAR_ITEM:
      snprintf(type, 32, "item");
      break;
    case BAR_COMPONENT_ALIAS:
      snprintf(type, 32, "alias");
      break;
    case BAR_COMPONENT_GROUP:
      snprintf(type, 32, "bracket");
      break;
    case BAR_COMPONENT_SLIDER:
      snprintf(type, 32, "slider");
      break;
    case BAR_COMPONENT_GRAPH:
      snprintf(type, 32, "graph");
      break;
    case BAR_COMPONENT_SPACE:
      snprintf(type, 32, "space");
      break;
    default:
      snprintf(type, 32, "invalid");
      break;
  }

  char position[32] = { 0 };
  switch (bar_item->position) {
    case POSITION_LEFT:
      snprintf(position, 32, "left");
      break;
    case POSITION_RIGHT:
      snprintf(position, 32, "right");
      break;
    case POSITION_CENTER:
      snprintf(position, 32, "center");
      break;
    case POSITION_CENTER_LEFT:
      snprintf(position, 32, "q");
      break;
    case POSITION_CENTER_RIGHT:
      snprintf(position, 32, "e");
      break;
    case POSITION_POPUP:
      snprintf(position, 32, "popup");
      break;
    default:
      snprintf(position, 32, "invalid");
      break;
  }

  fprintf(rsp, "{\n"
               "\t\"name\": \"%s\",\n"
               "\t\"type\": \"%s\",\n"
               "\t\"geometry\": {\n"
               "\t\t\"drawing\": \"%s\",\n"
               "\t\t\"position\": \"%s\",\n"
               "\t\t\"associated_space_mask\": %u,\n"
               "\t\t\"associated_display_mask\": %u,\n"
               "\t\t\"ignore_association\": \"%s\",\n"
               "\t\t\"y_offset\": %d,\n"
               "\t\t\"width\": %d,\n"
               "\t\t\"padding_left\": %d,\n"
               "\t\t\"padding_right\": %d,\n"
               "\t\t\"background\": {\n",
               bar_item->name,
               type,
               format_bool(bar_item->drawing),
               position,
               bar_item->associated_space,
               bar_item->associated_display,
               format_bool(bar_item->ignore_association),
               bar_item->y_offset,
               bar_item->background.padding_left,
               bar_item->background.padding_right,
               bar_item->has_const_width ? bar_item->custom_width : -1);

  background_serialize(&bar_item->background, "\t\t\t", rsp, true);
  fprintf(rsp, "\n\t\t}\n\t},\n");

  fprintf(rsp, "\t\"icon\": {\n");
  text_serialize(&bar_item->icon, "\t\t", rsp);
  fprintf(rsp, "\n\t},\n");

  fprintf(rsp, "\t\"label\": {\n");
  text_serialize(&bar_item->label, "\t\t", rsp);
  fprintf(rsp, "\n\t},\n");


  char* escaped_script = escape_string(bar_item->script);
  char* escaped_click_script = escape_string(bar_item->click_script);
  fprintf(rsp, "\t\"scripting\": {\n"
               "\t\t\"script\": \"%s\",\n"
               "\t\t\"click_script\": \"%s\",\n"
               "\t\t\"update_freq\": %u,\n"
               "\t\t\"update_mask\": %llu,\n"
               "\t\t\"updates\": \"%s\"\n\t},\n",
               escaped_script,
               escaped_click_script,
               bar_item->update_frequency,
               bar_item->update_mask,
               bar_item->updates_only_when_shown
                ? "when_shown"
                : format_bool(bar_item->updates) );

  if (escaped_script) free(escaped_script);
  if (escaped_click_script) free(escaped_click_script);

  fprintf(rsp, "\t\"bounding_rects\": {\n");
  int counter = 0;
  for (int i = 0; i < bar_item->num_windows; i++) {
    if (!bar_item->windows[i]) continue;
    if (counter++ > 0) fprintf(rsp, ",\n");
    fprintf(rsp, "\t\t\"display-%d\": {\n"
            "\t\t\t\"origin\": [ %f, %f ],\n"
            "\t\t\t\"size\": [ %f, %f ]\n\t\t}",
            i + 1,
            bar_item->windows[i]->origin.x,
            bar_item->windows[i]->origin.y,
            bar_item->windows[i]->frame.size.width,
            bar_item->windows[i]->frame.size.height);
  } 
  fprintf(rsp, "\n\t}");

  if (bar_item->popup.num_items > 0) {
    fprintf(rsp, ",\n\t\"popup\": {\n");
    popup_serialize(&bar_item->popup, "\t\t", rsp);
    fprintf(rsp, "\n\t}");
  }

  if (bar_item->type == BAR_COMPONENT_GROUP && bar_item->group) {
    fprintf(rsp, ",\n\t\"bracket\": [\n");
    group_serialize(bar_item->group, "\t\t", rsp);
    fprintf(rsp, "\n\t]");
  } else if (bar_item->type == BAR_COMPONENT_GRAPH) {
    fprintf(rsp, ",\n\t\"graph\": {\n");
    graph_serialize(&bar_item->graph, "\t\t", rsp);
    fprintf(rsp, "\n\t}");
  } else if (bar_item->type == BAR_COMPONENT_SLIDER) {
    fprintf(rsp, ",\n\t\"slider\": {\n");
    slider_serialize(&bar_item->slider, "\t\t", rsp);
    fprintf(rsp, "\n\t}");
  }

  fprintf(rsp, "\n}\n");
}

void bar_item_parse_set_message(struct bar_item* bar_item, char* message, FILE* rsp) {
  bool needs_refresh = false;
  struct token property = get_token(&message);

  struct key_value_pair key_value_pair = get_key_value_pair(property.text,'.');
  if (key_value_pair.key && key_value_pair.value) {
    struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
    struct token entry = { key_value_pair.value, strlen(key_value_pair.value)};
    if (token_equals(subdom, SUB_DOMAIN_ICON)) {
      needs_refresh = text_parse_sub_domain(&bar_item->icon,
                                            rsp,
                                            entry,
                                            message         );
    }
    else if (token_equals(subdom, SUB_DOMAIN_LABEL)) {
      needs_refresh = text_parse_sub_domain(&bar_item->label,
                                            rsp,
                                            entry,
                                            message          );
    }
    else if (token_equals(subdom, SUB_DOMAIN_BACKGROUND)) {
      needs_refresh = background_parse_sub_domain(&bar_item->background,
                                                  rsp,
                                                  entry,
                                                  message               );
    }
    else if (token_equals(subdom, SUB_DOMAIN_POPUP)) {
      needs_refresh = popup_parse_sub_domain(&bar_item->popup,
                                             rsp,
                                             entry,
                                             message          );
    }
    else if (token_equals(subdom, SUB_DOMAIN_GRAPH)) {
      if (bar_item->has_graph || bar_item == &g_bar_manager.default_item) {
        needs_refresh = graph_parse_sub_domain(&bar_item->graph,
                                               rsp,
                                               entry,
                                               message          );
      } else {
        respond(rsp, "[!] Item (%s): Trying to set a graph property on a non-graph item\n", bar_item->name);
      }
    }
    else if (token_equals(subdom, SUB_DOMAIN_ALIAS)) {
      if (bar_item->has_alias || bar_item == &g_bar_manager.default_item) {
        needs_refresh = alias_parse_sub_domain(&bar_item->alias,
                                               rsp,
                                               entry,
                                               message          );
      } else {
        respond(rsp, "[!] Item (%s): Trying to set an alias property on a non-alias item\n", bar_item->name);
      }
    }
    else if (token_equals(subdom, SUB_DOMAIN_SLIDER)) {
      if (bar_item->has_slider || bar_item == &g_bar_manager.default_item) {
        needs_refresh = slider_parse_sub_domain(&bar_item->slider,
                                                rsp,
                                                entry,
                                                message           );
      } else {
        respond(rsp, "[!] Item (%s): Trying to set a slider property on a non-slider item\n", bar_item->name);
      }
    }
    else {
      respond(rsp, "[!] Item (%s): Invalid subdomain '%s'\n", bar_item->name, subdom.text);
    }
  }
  else if (token_equals(property, PROPERTY_ICON)) {
    struct token dummy = { PROPERTY_STRING, strlen(PROPERTY_STRING)};
    needs_refresh = text_parse_sub_domain(&bar_item->icon,
                                          rsp,
                                          dummy,
                                          message         );

  } else if (token_equals(property, PROPERTY_LABEL)) {
    struct token dummy = { PROPERTY_STRING, strlen(PROPERTY_STRING)};
    needs_refresh = text_parse_sub_domain(&bar_item->label,
                                          rsp,
                                          dummy,
                                          message          );

  } else if (token_equals(property, PROPERTY_UPDATES)) {
    struct token token = get_token(&message);
    if (token_equals(token, ARGUMENT_UPDATES_WHEN_SHOWN)) {
      bar_item->updates = true;
      bar_item->updates_only_when_shown = true;
    }
    else {
      bar_item->updates = evaluate_boolean_state(token, bar_item->updates);
      bar_item->updates_only_when_shown = false;
    }
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    needs_refresh = bar_item_set_drawing(bar_item,
                                         evaluate_boolean_state(get_token(&message),
                                                                bar_item->drawing   ));
  } else if (token_equals(property, PROPERTY_SCROLL_TEXTS)) {
    bar_item->scroll_texts = evaluate_boolean_state(get_token(&message),
                                                    bar_item->scroll_texts);
  } else if (token_equals(property, PROPERTY_WIDTH)) {
    struct token token = get_token(&message);
    if (token_equals(token, ARGUMENT_DYNAMIC)) {
      ANIMATE(bar_item_set_width,
              bar_item,
              bar_item->custom_width,
              bar_item_get_length(bar_item, true)
              + bar_item->background.padding_left
              + bar_item->background.padding_right);

      struct animation* animation = animation_create();
      animation_setup(animation,
                      bar_item,
                      (bool (*)(void*, int))&bar_item_set_width,
                      bar_item->custom_width,
                      -1,
                      1,
                      INTERP_FUNCTION_LINEAR               );
      animator_add(&g_bar_manager.animator, animation);
    }
    else {
      ANIMATE(bar_item_set_width,
              bar_item,
              bar_item_get_length(bar_item, false)
              + (bar_item->has_const_width
              ? 0
              : (bar_item->background.padding_left
                 + bar_item->background.padding_right)),
              token_to_int(token)                       );
    }
  } else if (token_equals(property, PROPERTY_SCRIPT)) {
    bar_item_set_script(bar_item, token_to_string(get_token(&message)));
  } else if (token_equals(property, PROPERTY_CLICK_SCRIPT)) {
    bar_item_set_click_script(bar_item, token_to_string(get_token(&message)));
  } else if (token_equals(property, PROPERTY_UPDATE_FREQ)) {
    bar_item->update_frequency = token_to_uint32t(get_token(&message));
  } else if (token_equals(property, PROPERTY_POSITION)) {
    struct token position = get_token(&message);
    bar_item_set_position(bar_item, position.text);
    struct key_value_pair key_value_pair = get_key_value_pair(position.text,
                                                              '.'           );

    if (key_value_pair.key && key_value_pair.value) {
      if (key_value_pair.key[0] == POSITION_POPUP) {
        int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                      key_value_pair.value);
        if (item_index_for_name < 0) {
          respond(rsp, "[!] Item Position (%s): Item '%s' is not a valid popup host\n", bar_item->name, key_value_pair.value);
          return;
        }
        struct bar_item* target_item = g_bar_manager.bar_items[item_index_for_name];
        popup_add_item(&target_item->popup, bar_item);
      } else {
        bar_item->parent = NULL;
      }
    }
    needs_refresh = true;
  } else if (token_equals(property, PROPERTY_ALIGN)) {
    struct token position = get_token(&message);
    if (bar_item->align != position.text[0]) {
      bar_item->align = position.text[0];
      needs_refresh = true;
    }
  } else if (token_equals(property, PROPERTY_ASSOCIATED_SPACE)
             || token_equals(property, PROPERTY_SPACE)        ) {
    struct token token = get_token(&message);
    uint32_t prev = bar_item->associated_space;
    bar_item->associated_space = 0;
    uint32_t count;
    char** list = token_split(token, ',', &count);
    if (list && count > 0) {
      for (int i = 0; i < count; i++) {
        bar_item_append_associated_space(bar_item,
                                         1 << strtoul(list[i],
                                                      NULL,
                                                      0       ));
      }
      free(list);
    }
    needs_refresh = (prev != bar_item->associated_space);
  } else if (token_equals(property, PROPERTY_ASSOCIATED_DISPLAY)
             || token_equals(property, PROPERTY_DISPLAY)        ) {
    struct token token = get_token(&message);
    uint32_t prev = bar_item->associated_display;
    bar_item->associated_display = 0;
    bar_item->associated_to_active_display = false;
    uint32_t count;
    char** list = token_split(token, ',', &count);
    if (list && count > 0) {
      for (int i = 0; i < count; i++) {
        if (strcmp(list[i], "active") == 0) {
          bar_item->associated_to_active_display = true;
        }
        else {
          bar_item_append_associated_display(bar_item,
                                             1 << strtoul(list[i],
                                                          NULL,
                                                          0       ));
        }
      }
      free(list);
    }
    needs_refresh = (prev != bar_item->associated_display);
  } else if (token_equals(property, PROPERTY_YOFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(bar_item_set_yoffset,
            bar_item,
            bar_item->y_offset,
            token_to_int(token)  );

  } else if (token_equals(property, PROPERTY_PADDING_LEFT)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_padding_left,
            &bar_item->background,
            bar_item->background.padding_left,
            token_to_int(token)               );

  } else if (token_equals(property, PROPERTY_PADDING_RIGHT)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_padding_right,
            &bar_item->background,
            bar_item->background.padding_right,
            token_to_int(token)                );

  } else if (token_equals(property, PROPERTY_BLUR_RADIUS)) {
    struct token token = get_token(&message);
    ANIMATE(bar_item_set_blur_radius,
            bar_item,
            bar_item->blur_radius,
            token_to_int(token)      );

  } else if (token_equals(property, PROPERTY_SHADOW)) {
    bool prev = bar_item->shadow;
    bar_item->shadow = evaluate_boolean_state(get_token(&message),
                                              bar_item->shadow    );
    if (prev != bar_item->shadow) {
      for (int i = 1; i <= bar_item->num_windows; i++) {
        bar_item_remove_window(bar_item, i);
      }
      needs_refresh = true;
    }
  } else if (token_equals(property, PROPERTY_IGNORE_ASSOCIATION)) {
    bar_item->ignore_association = evaluate_boolean_state(get_token(&message),
                                                          bar_item->ignore_association);
    needs_refresh = true;
  } else if (token_equals(property, COMMAND_DEFAULT_RESET)) {
    bar_item_init(&g_bar_manager.default_item, NULL);
  } else if (token_equals(property, PROPERTY_EVENT_PORT)) {
    struct token token = get_token(&message);
    if (token.text && token.length > 0)
      bar_item_set_event_port(bar_item, token.text);
  } else {
    respond(rsp, "[!] Item (%s): Invalid property '%s' \n", bar_item->name, property.text);
  }

  if (needs_refresh) bar_item_needs_update(bar_item);
}

void bar_item_parse_subscribe_message(struct bar_item* bar_item, char* message, FILE* rsp) {
  struct token event = get_token(&message);

  while (event.text && event.length > 0) {
    uint64_t event_flag = custom_events_get_flag_for_name(&g_bar_manager.custom_events,
                                                          event.text                   );

    if (event_flag & UPDATE_VOLUME_CHANGE) {
      begin_receiving_volume_events();
    }

    if (event_flag & UPDATE_BRIGHTNESS_CHANGE) {
      begin_receiving_brightness_events();
    }

    if (event_flag & UPDATE_MEDIA_CHANGE) {
      begin_receiving_media_events();
    }

    if (event_flag & UPDATE_SPACE_WINDOWS_CHANGE) {
      begin_receiving_space_window_events();
    }

    bar_item->update_mask |= event_flag;

    if (!event_flag) {
      respond(rsp, "[?] Event: '%s' not found\n", event.text);
    }

    event = get_token(&message);
  }
}
