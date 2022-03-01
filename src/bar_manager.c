#include "bar_manager.h"
#include "background.h"
#include "bar.h"
#include "bar_item.h"
#include "misc/env_vars.h"
#include "misc/helpers.h"
#include "popup.h"

extern struct event_loop g_event_loop;

static CLOCK_CALLBACK(clock_handler) {
  struct event *event = event_create(&g_event_loop, SHELL_REFRESH, NULL);
  event_loop_post(&g_event_loop, event);
}

void bar_manager_init(struct bar_manager* bar_manager) {
  bar_manager->font_smoothing = false;
  bar_manager->any_bar_hidden = false;
  bar_manager->bars = NULL;
  bar_manager->bar_count = 0;
  bar_manager->bar_item_count = 0;
  bar_manager->display = DISPLAY_ALL;
  bar_manager->position = POSITION_TOP;
  bar_manager->y_offset = 0;
  bar_manager->shadow = false;
  bar_manager->blur_radius = 0;
  bar_manager->margin = 0;
  bar_manager->frozen = false;
  bar_manager->sleeps = false;
  bar_manager->window_level = NSNormalWindowLevel;
  bar_manager->topmost = false;
  bar_manager->picky_redraw = false;
  bar_manager->notch_width = 200;
  bar_manager->active_adid = display_arrangement(display_active_display_id());

  background_init(&bar_manager->background);
  bar_manager->background.bounds.size.height = 25;
  bar_manager->background.padding_left = 20;
  bar_manager->background.padding_right = 20;
  bar_manager->background.border_color = rgba_color_from_hex(0xffff0000);
  bar_manager->background.color = rgba_color_from_hex(0x44000000);

  bar_item_init(&bar_manager->default_item, NULL);
  bar_manager->default_item.name = string_copy("defaults");
  custom_events_init(&bar_manager->custom_events);
  
  int shell_refresh_frequency = 1;

  bar_manager->clock = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + shell_refresh_frequency, shell_refresh_frequency, 0, 0, clock_handler, NULL);
  CFRunLoopAddTimer(CFRunLoopGetMain(), bar_manager->clock, kCFRunLoopCommonModes);
}

void bar_manager_sort(struct bar_manager* bar_manager, struct bar_item** ordering, uint32_t count) {
  int index = 0;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    for (int j = 0; j < count; j++) {
      if (bar_manager->bar_items[i] == ordering[j] && bar_manager->bar_items[i] != ordering[index]) {
        bar_manager->bar_items[i] = ordering[index];
        index++;
        bar_item_needs_update(bar_manager->bar_items[i]);
        break;
      }
      else if (bar_manager->bar_items[i] == ordering[j] && bar_manager->bar_items[i] == ordering[index]) {
        index++;
        break;
      }
    }
  }
}

int bar_manager_get_item_index_for_name(struct bar_manager* bar_manager, char* name) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    if (strcmp(bar_manager->bar_items[i]->name, name) == 0) {
      return i;
    }
  }
  return -1;
}

int bar_manager_get_item_index_by_address(struct bar_manager* bar_manager, struct bar_item* bar_item) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    if (bar_manager->bar_items[i] == bar_item) {
      return i;
    }
  }
  return -1;
}

void bar_manager_move_item(struct bar_manager* bar_manager, struct bar_item* item, struct bar_item* reference, bool before) {
  if (bar_manager->bar_item_count <= 0) return;
  struct bar_item* tmp[bar_manager->bar_item_count];
  int count = 0;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    if (bar_manager->bar_items[i] == item) continue;
    if (bar_manager->bar_items[i] == reference && before) {
      tmp[count++] = item;
      tmp[count++] = bar_manager->bar_items[i];
      continue;
    } else if (bar_manager->bar_items[i] == reference && !before) {
      tmp[count++] = bar_manager->bar_items[i];
      tmp[count++] = item;
      continue;
    }
    tmp[count++] = bar_manager->bar_items[i];
  }
  bar_manager->bar_items = realloc(bar_manager->bar_items, sizeof(struct bar_item*)*bar_manager->bar_item_count);
  memcpy(bar_manager->bar_items, tmp, sizeof(struct bar_item*)*bar_manager->bar_item_count);
}

void bar_manager_remove_item(struct bar_manager* bar_manager, struct bar_item* bar_item) {
  if (bar_manager->bar_item_count <= 0 || !bar_item || bar_manager_get_item_index_by_address(bar_manager, bar_item) < 0) return;
  if (bar_item->position == POSITION_POPUP) {
    for (int i = 0; i < bar_manager->bar_item_count; i++) {
      popup_remove_item(&bar_manager->bar_items[i]->popup, bar_item);
    }
  }
  if (bar_manager->bar_item_count == 1) {
    free(bar_manager->bar_items);
    bar_manager->bar_items = NULL;
    bar_manager->bar_item_count = 0;
  } else {
    struct bar_item* tmp[bar_manager->bar_item_count - 1];
    int count = 0;
    for (int i = 0; i < bar_manager->bar_item_count; i++) {
      if (bar_manager->bar_items[i] == bar_item) continue;
      tmp[count++] = bar_manager->bar_items[i];
    }
    bar_manager->bar_item_count--;
    bar_manager->bar_items = realloc(bar_manager->bar_items, sizeof(struct bar_item*)*bar_manager->bar_item_count);
    memcpy(bar_manager->bar_items, tmp, sizeof(struct bar_item*)*bar_manager->bar_item_count);
  }
  bar_item_destroy(bar_item);
}

bool bar_manager_set_background_blur(struct bar_manager* bar_manager, uint32_t radius) {
  if (bar_manager->blur_radius == radius) return false;
  bar_manager->blur_radius = radius;
  for (int i = 0; i < bar_manager->bar_count; i++) {
    window_set_blur_radius(bar_manager->bars[i]->id);
  }
  return true;
}

bool bar_manager_set_position(struct bar_manager* bar_manager, char pos) {
  if (bar_manager->position == pos) return false;
  bar_manager->position = pos;
  return true;
}

bool bar_manager_set_display(struct bar_manager* bar_manager, char display) {
  if (bar_manager->display == display) return false;
  bar_manager->display = display;

  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_destroy(bar_manager->bars[i]);

  bar_manager_begin(bar_manager);
  return true;
}

bool bar_manager_set_shadow(struct bar_manager* bar_manager, bool shadow) {
  if (bar_manager->shadow == shadow) return false;
  bar_manager->shadow = shadow;
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_destroy(bar_manager->bars[i]);

  bar_manager_begin(bar_manager);
  return true;
}

bool bar_manager_set_notch_width(struct bar_manager* bar_manager, uint32_t width) {
  if (bar_manager->notch_width == width) return false;
  bar_manager->notch_width = width;
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_destroy(bar_manager->bars[i]);

  bar_manager_begin(bar_manager);
  return true;
}

bool bar_manager_set_font_smoothing(struct bar_manager* bar_manager, bool smoothing) {
  if (bar_manager->font_smoothing == smoothing) return false;
  bar_manager->font_smoothing = smoothing;
  for (int i = 0; i < bar_manager->bar_count; i++)
    context_set_font_smoothing(bar_manager->bars[i]->context, smoothing);
  return true;
}

bool bar_manager_set_hidden(struct bar_manager *bar_manager, uint32_t adid, bool hidden) {
  bar_manager->any_bar_hidden = false;
  if (adid > 0) {
    bar_set_hidden(bar_manager->bars[adid - 1], hidden);
    bar_manager->any_bar_hidden |= hidden;
  }
  else {
    for (int i = 0; i < bar_manager->bar_count; i++) {
      bar_set_hidden(bar_manager->bars[i], hidden);
      bar_manager->any_bar_hidden |= hidden;
    }
  }

  if (hidden) {
    for (int i = 0; i < bar_manager->bar_item_count; i++) {
      popup_set_drawing(&bar_manager->bar_items[i]->popup, false);
    }
  }
  return true;
}

bool bar_manager_set_topmost(struct bar_manager *bar_manager, bool topmost) {
  for (int i = 0; i < bar_manager->bar_count; i++) bar_destroy(bar_manager->bars[i]);
  if (topmost) bar_manager->window_level = NSScreenSaverWindowLevel;
  else bar_manager->window_level = NSNormalWindowLevel;
  bar_manager_begin(bar_manager);
  bar_manager->topmost = topmost;
  return true;
}

void bar_manager_freeze(struct bar_manager *bar_manager) {
  bar_manager->frozen = true;
}

void bar_manager_unfreeze(struct bar_manager *bar_manager) {
  bar_manager->frozen = false;
}

uint32_t bar_manager_length_for_bar_side(struct bar_manager* bar_manager, struct bar* bar, char side) {
  uint32_t total_length = 0;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->position == side && bar_draws_item(bar, bar_item))
      total_length += bar_item_get_length(bar_item, false) + (bar_item->has_const_width ? 0 : bar_item->background.padding_left + bar_item->background.padding_right);
  }
  return total_length;
}

bool bar_manager_bar_needs_redraw(struct bar_manager* bar_manager, struct bar* bar) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    bool is_associated_space_shown = (bar_item->associated_space & (1 << bar->sid)) || bar_item->associated_space == 0;
    bool is_associated_display_shown = (bar_item->associated_display & (1 << bar->adid));
    if ((bar_item->drawing || (!bar_item->drawing && bar_item->associated_bar != 0)) && bar_item->needs_update && (is_associated_space_shown || is_associated_display_shown) && !bar_item->lazy)
      return true;
  }
  return false;
}

void bar_manager_clear_needs_update(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) 
    bar_item_clear_needs_update(bar_manager->bar_items[i]);
}

void bar_manager_clear_association_for_bar(struct bar_manager* bar_manager, struct bar* bar) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) 
    bar_item_remove_associated_bar(bar_manager->bar_items[i], bar->adid);
}

void bar_manager_reset_bar_association(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) 
    bar_item_reset_associated_bar(bar_manager->bar_items[i]);
}

void bar_manager_refresh(struct bar_manager* bar_manager, bool forced) {
  if (bar_manager->frozen) return;
  if (forced) bar_manager_reset_bar_association(bar_manager);
  for (int i = 0; i < bar_manager->bar_count; ++i) {
    if (forced || bar_manager_bar_needs_redraw(bar_manager, bar_manager->bars[i])) { 
      bar_calculate_bounds(bar_manager->bars[i]);
      bar_draw(bar_manager->bars[i]);
    }
  }
  bar_manager_clear_needs_update(bar_manager);
}

void bar_manager_resize(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_resize(bar_manager->bars[i]);
}

struct bar_item* bar_manager_create_item(struct bar_manager* bar_manager) {
  bar_manager->bar_items = (struct bar_item**) realloc(bar_manager->bar_items, sizeof(struct bar_item*) * (bar_manager->bar_item_count + 1));
  bar_manager->bar_item_count += 1;
  struct bar_item* bar_item = bar_item_create();
  bar_item_init(bar_item, &bar_manager->default_item);
  bar_manager->bar_items[bar_manager->bar_item_count - 1] = bar_item;
  return bar_item;
}

void bar_manager_update_alias_components(struct bar_manager* bar_manager, bool forced) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    if ((!bar_item_is_shown(bar_manager->bar_items[i]) && !forced) || bar_manager->bar_items[i]->type != BAR_COMPONENT_ALIAS) continue;
    bar_item_update(bar_manager->bar_items[i], NULL, forced, NULL);
  }
}

void bar_manager_update_space_components(struct bar_manager* bar_manager, bool forced) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->type != BAR_COMPONENT_SPACE) continue;

    for (int j = 0; j < bar_manager->bar_count; j++) {
      struct bar* bar = bar_manager->bars[j];
      uint32_t did = bar->adid;

      if (((1 << did) & bar_item->associated_display)) {
        uint32_t sid = bar->sid;
        if (sid == 0) continue;
        if ((!bar_item->selected || forced) && bar_item->associated_space & (1 << sid)) {
          bar_item->selected = true;
          bar_item->updates = true;
          env_vars_set(&bar_item->signal_args.env_vars, string_copy("SELECTED"), string_copy("true"));
        }
        else if ((bar_item->selected || forced) && !(bar_item->associated_space & (1 << sid))) {
          bar_item->selected = false;
          bar_item->updates = true;
          env_vars_set(&bar_item->signal_args.env_vars, string_copy("SELECTED"), string_copy("false"));
        }
        else {
          bar_item->updates = false;
        }
      } 
    }
  }
}

void bar_manager_update(struct bar_manager* bar_manager, bool forced) {
  if ((bar_manager->frozen && !forced) || bar_manager->sleeps) return;
  bool needs_refresh = false;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    needs_refresh |= bar_item_update(bar_manager->bar_items[i], NULL, forced, NULL);
  }
  if (needs_refresh) bar_manager_refresh(bar_manager, false);
}

void bar_manager_begin(struct bar_manager *bar_manager) {
  if (bar_manager->display == DISPLAY_MAIN) {
    uint32_t did = display_main_display_id();
    bar_manager->bar_count = 1;
    bar_manager->bars = (struct bar **) realloc(bar_manager->bars, sizeof(struct bar *) * bar_manager->bar_count);
    memset(bar_manager->bars, 0, sizeof(struct bar*) * bar_manager->bar_count);
    bar_manager->bars[0] = bar_create(did);
    bar_manager->bars[0]->adid = 1;
  } 
  else {
    bar_manager->bar_count = display_active_display_count();
    bar_manager->bars = (struct bar **) realloc(bar_manager->bars, sizeof(struct bar *) * bar_manager->bar_count);
    memset(bar_manager->bars, 0, sizeof(struct bar*) * bar_manager->bar_count);
    for (uint32_t index=1; index <= bar_manager->bar_count; index++) {
      uint32_t did = display_arrangement_display_id(index);
      bar_manager->bars[index - 1] = bar_create(did);
      bar_manager->bars[index - 1]->adid = index;
    }
  }
}

struct bar_item* bar_manager_get_item_by_point(struct bar_manager* bar_manager, CGPoint point, uint32_t adid) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (!bar_item->drawing || bar_item->num_rects < adid || bar_item->bounding_rects[adid - 1] == NULL) continue;
    if (cgrect_contains_point(bar_item->bounding_rects[adid - 1], &point)) {
      return bar_item;
    }
  }
  return NULL;
}

void bar_manager_custom_events_trigger(struct bar_manager* bar_manager, char* name, struct env_vars* env_vars) {
  uint64_t flag = custom_events_get_flag_for_name(&bar_manager->custom_events, name);

  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->update_mask & flag)
      bar_item_update(bar_item, name, false, env_vars);
  }
}

void bar_manager_display_changed(struct bar_manager* bar_manager) {
  bar_manager->active_adid = display_arrangement(display_active_display_id());

  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_destroy(bar_manager->bars[i]);

  bar_manager_begin(bar_manager);
  bar_manager_refresh(bar_manager, true);
}

void bar_manager_handle_mouse_entered(struct bar_manager* bar_manager, struct bar_item* bar_item) {
  if (!bar_item || bar_item->mouse_over) return;
  for (int i = 0; i < bar_manager->bar_item_count; i++)
    bar_item_mouse_exited(bar_item);

  bar_item_mouse_entered(bar_item);
}

void bar_manager_handle_mouse_exited(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++)
    bar_item_mouse_exited(bar_manager->bar_items[i]);
}

void bar_manager_handle_front_app_switch(struct bar_manager* bar_manager, char* info) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  if (info) env_vars_set(&env_vars, string_copy("INFO"), info);
  bar_manager_custom_events_trigger(bar_manager, COMMAND_SUBSCRIBE_FRONT_APP_SWITCHED, &env_vars);
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_space_change(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_count; i++) {
    bar_manager->bars[i]->sid = mission_control_index(display_space_id(bar_manager->bars[i]->did));
    bar_manager->bars[i]->shown = SLSSpaceGetType(g_connection, bar_manager->bars[i]->sid) != 4;

  }
  bar_manager_update_space_components(bar_manager, false);
  bar_manager_custom_events_trigger(bar_manager, COMMAND_SUBSCRIBE_SPACE_CHANGE, NULL);
  bar_manager_refresh(bar_manager, true);
}

void bar_manager_handle_display_change(struct bar_manager* bar_manager) {
  bar_manager->active_adid = display_arrangement(display_active_display_id());
  bar_manager_custom_events_trigger(bar_manager, COMMAND_SUBSCRIBE_DISPLAY_CHANGE, NULL);
}

void bar_manager_handle_system_will_sleep(struct bar_manager* bar_manager) {
  bar_manager_custom_events_trigger(bar_manager, COMMAND_SUBSCRIBE_SYSTEM_WILL_SLEEP, NULL);
  bar_manager->sleeps = true;
}

void bar_manager_handle_system_woke(struct bar_manager* bar_manager) {
  bar_manager->sleeps = false;
  bar_manager_update_space_components(bar_manager, false);
  bar_manager_custom_events_trigger(bar_manager, COMMAND_SUBSCRIBE_SYSTEM_WOKE, NULL);
  bar_manager_refresh(bar_manager, true);
}

void bar_manager_handle_notification(struct bar_manager* bar_manager, struct notification* notification) {
  char* name = custom_events_get_name_for_notification(&bar_manager->custom_events, notification->name);
  if (!name) {
    notification_destroy(notification);
    return;
  }

  struct env_vars env_vars;
  env_vars_init(&env_vars);
  if (notification->info) env_vars_set(&env_vars, string_copy("INFO"), string_copy(notification->info));
  bar_manager_custom_events_trigger(bar_manager, name, &env_vars);
  env_vars_destroy(&env_vars);
  notification_destroy(notification);
}

void bar_manager_destroy(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    bar_item_destroy(bar_manager->bar_items[i]);
  }
  if (bar_manager->bar_items) free(bar_manager->bar_items);
  for (int i = 0; i < bar_manager->bar_count; i++) {
    bar_destroy(bar_manager->bars[i]);
  }
  custom_events_destroy(&bar_manager->custom_events);
  background_destroy(&bar_manager->background);

  if (bar_manager->bars) free(bar_manager->bars);
}

void bar_manager_serialize(struct bar_manager* bar_manager, FILE* rsp) {
  fprintf(rsp, "{\n"
               "\t\"geometry\": {\n"
               "\t\t\"position\": \"%c\",\n"
               "\t\t\"height\": %u,\n"
               "\t\t\"margin\": %d,\n"
               "\t\t\"y_offset\": %d,\n"
               "\t\t\"corner_radius\": %u,\n"
               "\t\t\"border_width\": %u,\n"
               "\t\t\"padding_left\": %u,\n"
               "\t\t\"padding_right\": %u\n"
               "\t},\n"
               "\t\"style\": {\n"
               "\t\t\"color\": \"0x%x\",\n"
               "\t\t\"border_color\": \"0x%x\",\n"
               "\t\t\"blur_radius\": %u\n"
               "\t},\n"
               "\t\"state\": {\n"
               "\t\t\"frozen\": %d,\n"
               "\t\t\"topmost\": %d,\n"
               "\t\t\"shadow\": %d,\n"
               "\t\t\"font_smoothing\": %d\n"
               "\t},\n"
               "\t\"items\": [\n",
               bar_manager->position,
               (uint32_t)bar_manager->background.bounds.size.height,
               bar_manager->margin,
               bar_manager->y_offset,
               bar_manager->background.corner_radius,
               bar_manager->background.border_width,
               bar_manager->background.padding_left,
               bar_manager->background.padding_right,
               hex_from_rgba_color(bar_manager->background.color),
               hex_from_rgba_color(bar_manager->background.border_color),
               bar_manager->blur_radius,
               bar_manager->frozen,
               bar_manager->topmost,
               bar_manager->shadow,
               bar_manager->font_smoothing);
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    fprintf(rsp, "\t\t \"%s\"", bar_manager->bar_items[i]->name);
    if (i < bar_manager->bar_item_count - 1) fprintf(rsp, ",\n");
  }
  fprintf(rsp, "\n\t]\n}\n");
}
