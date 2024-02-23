#include "bar_manager.h"
#include "bar.h"
#include "bar_item.h"
#include "event.h"
#include "misc/env_vars.h"
#include "misc/helpers.h"
#include "wifi.h"
#include "volume.h"
#include "power.h"
#include "mouse.h"
#include "media.h"
#include "app_windows.h"

extern void forced_front_app_event();

static CLOCK_CALLBACK(clock_handler) {
  struct event event = { NULL, SHELL_REFRESH };
  event_post(&event);
}

void bar_manager_init(struct bar_manager* bar_manager) {
  bar_manager->font_smoothing = false;
  bar_manager->any_bar_hidden = false;
  bar_manager->needs_ordering = false;
  bar_manager->bar_needs_update = false;
  bar_manager->bars = NULL;
  bar_manager->bar_count = 0;
  bar_manager->bar_items = NULL;
  bar_manager->bar_item_count = 0;
  bar_manager->displays = DISPLAY_ALL_PATTERN;
  bar_manager->position = POSITION_TOP;
  bar_manager->shadow = false;
  bar_manager->blur_radius = 0;
  bar_manager->margin = 0;
  bar_manager->frozen = false;
  bar_manager->sleeps = false;
  bar_manager->window_level = kCGBackstopMenuLevel;
  bar_manager->topmost = false;
  bar_manager->notch_width = 200;
  bar_manager->notch_offset = 0;
  bar_manager->active_adid = display_active_display_adid();
  bar_manager->might_need_clipping = false;

  bar_manager->sticky = true;

  image_init(&bar_manager->current_artwork);
  background_init(&bar_manager->background);
  bar_manager->background.bounds.size.height = 25;
  bar_manager->background.overrides_height = true;
  bar_manager->background.padding_left = 20;
  bar_manager->background.padding_right = 20;

  color_set_hex(&bar_manager->background.border_color, 0xffff0000);
  color_set_hex(&bar_manager->background.color, 0x44000000);

  bar_item_init(&bar_manager->default_item, NULL);
  bar_item_set_name(&bar_manager->default_item, string_copy("defaults"));
  custom_events_init(&bar_manager->custom_events);

  animator_init(&bar_manager->animator);
  
  int shell_refresh_frequency = 1;

  bar_manager->clock = CFRunLoopTimerCreate(NULL,
                                            CFAbsoluteTimeGetCurrent()
                                            + shell_refresh_frequency,
                                            shell_refresh_frequency,
                                            0,
                                            0,
                                            clock_handler,
                                            NULL                      );

  CFRunLoopAddTimer(CFRunLoopGetMain(),
                    bar_manager->clock,
                    kCFRunLoopCommonModes);
}

void bar_manager_sort(struct bar_manager* bar_manager, struct bar_item** ordering, uint32_t count) {
  int index = 0;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    for (int j = 0; j < count; j++) {
      if (bar_manager->bar_items[i] == ordering[j]
          && bar_manager->bar_items[i] != ordering[index]) {
        bar_manager->bar_items[i] = ordering[index];
        index++;
        bar_item_needs_update(bar_manager->bar_items[i]);
        break;
      }
      else if (bar_manager->bar_items[i] == ordering[j]
               && bar_manager->bar_items[i] == ordering[index]) {
        index++;
        break;
      }
    }
  }
  bar_manager->needs_ordering = true;
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
  bar_manager->bar_items = realloc(
                        bar_manager->bar_items,
                        sizeof(struct bar_item*)*bar_manager->bar_item_count);

  memcpy(bar_manager->bar_items,
         tmp,
         sizeof(struct bar_item*)*bar_manager->bar_item_count);

  bar_manager->needs_ordering = true;
}

void bar_manager_remove_item(struct bar_manager* bar_manager, struct bar_item* bar_item) {
  if (bar_manager->bar_item_count <= 0 || !bar_item
      || bar_manager_get_item_index_by_address(bar_manager, bar_item) < 0) {
    return;
  }

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
    bar_manager->bar_items = realloc(
                         bar_manager->bar_items,
                         sizeof(struct bar_item*)*bar_manager->bar_item_count);

    memcpy(bar_manager->bar_items,
           tmp,
           sizeof(struct bar_item*)*bar_manager->bar_item_count);
  }

  bar_item_destroy(bar_item, true);
}

bool bar_manager_set_margin(struct bar_manager* bar_manager, int margin) {
  if (bar_manager->margin == margin) return false;
  bar_manager->margin = margin;
  bar_manager->bar_needs_resize = true;
  return true;
}

bool bar_manager_set_y_offset(struct bar_manager* bar_manager, int y_offset) {
  if (bar_manager->background.y_offset == y_offset) return false;
  bar_manager->background.y_offset = y_offset;
  bar_manager->bar_needs_resize = true;
  return true;
}

bool bar_manager_set_bar_height(struct bar_manager* bar_manager, int height) {
  bar_manager->bar_needs_resize |= background_set_height(&bar_manager->background, height);
  return bar_manager->bar_needs_resize;
}

bool bar_manager_set_background_blur(struct bar_manager* bar_manager, uint32_t radius) {
  if (bar_manager->blur_radius == radius) return false;
  bar_manager->blur_radius = radius;
  for (int i = 0; i < bar_manager->bar_count; i++) {
    window_set_blur_radius(&bar_manager->bars[i]->window, radius);
  }
  return false;
}

bool bar_manager_set_position(struct bar_manager* bar_manager, char pos) {
  if (bar_manager->position == pos) return false;
  bar_manager->position = pos;
  bar_manager->bar_needs_resize = true;
  return true;
}

bool bar_manager_set_displays(struct bar_manager* bar_manager, uint32_t displays) {
  if (bar_manager->displays == displays) return false;
  bar_manager->displays = displays;

  bar_manager_reset(bar_manager);
  return true;
}

bool bar_manager_set_shadow(struct bar_manager* bar_manager, bool shadow) {
  if (bar_manager->shadow == shadow) return false;
  bar_manager->shadow = shadow;
  bar_manager_reset(bar_manager);
  return true;
}

bool bar_manager_set_notch_width(struct bar_manager* bar_manager, uint32_t width) {
  if (bar_manager->notch_width == width) return false;

  bar_manager->notch_width = width;
  return true;
}

bool bar_manager_set_notch_offset(struct bar_manager* bar_manager, uint32_t offset) {
  if (bar_manager->notch_offset == offset) return false;

  bar_manager->notch_offset = offset;
  bar_manager->bar_needs_resize = true;
  return true;
}

bool bar_manager_set_font_smoothing(struct bar_manager* bar_manager, bool smoothing) {
  if (bar_manager->font_smoothing == smoothing) return false;
  bar_manager->font_smoothing = smoothing;
  for (int i = 0; i < bar_manager->bar_count; i++)
    context_set_font_smoothing(bar_manager->bars[i]->window.context, smoothing);
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

  bar_manager->bar_needs_update = true;
  return true;
}

bool bar_manager_set_topmost(struct bar_manager *bar_manager, char level, bool topmost) {
  if (topmost) {
    if (level == TOPMOST_LEVEL_WINDOW) {
      bar_manager->window_level = kCGFloatingWindowLevel;
    } else if (level == TOPMOST_LEVEL_ALL) {
      bar_manager->window_level = kCGStatusWindowLevel;
    }
  } else {
    bar_manager->window_level = kCGBackstopMenuLevel;
  }

  bar_manager_reset(bar_manager);
  bar_manager->topmost = topmost;
  return true;
}

bool bar_manager_set_sticky(struct bar_manager *bar_manager, bool sticky) {
  if (sticky == bar_manager->sticky) return false;

  bar_manager->sticky = sticky;
  bar_manager_reset(bar_manager);
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
    if (bar_item->position == side
        && bar_item->type != BAR_COMPONENT_GROUP
        && bar_draws_item(bar, bar_item)        ) {
      int item_length = (bar_manager->position == POSITION_LEFT
                         || bar_manager->position == POSITION_RIGHT)
                        ? bar_item_get_height(bar_item)
                        : bar_item_get_length(bar_item, false);

      total_length += item_length + (bar_item->has_const_width
                                     ? 0
                                     : bar_item->background.padding_left
                                       + bar_item->background.padding_right);
    }
  }
  return total_length;
}

bool bar_manager_bar_needs_redraw(struct bar_manager* bar_manager, struct bar* bar) {
  if (bar_manager->bar_needs_update) return true;

  uint32_t bar_mask = 1 << bar->adid;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    bool draws_item = bar_draws_item(bar, bar_item);
    
    bool regular_update = bar_item->needs_update
                          && draws_item;

    if (regular_update) return true;

    bool disabled_item_drawn_on_bar = !bar_item->drawing
                                      && (bar_item->associated_bar != 0);

    if (disabled_item_drawn_on_bar) return true;

    if (bar_item->ignore_association) continue;

    bool not_drawn_on_associated_display =
      (draws_item
       && (bar_item->associated_display > 0)
       && (bar_item->associated_display & bar_mask)
       && !(((bar_item->associated_bar << 1) & bar_mask)))
      || (draws_item
          && bar_item->associated_to_active_display
          && bar_manager->active_adid == bar->adid
          && !((bar_item->associated_bar << 1) & bar_mask));

    if (not_drawn_on_associated_display) return true;

    bool drawn_on_non_associated_display =
      (!bar_item->associated_to_active_display
       && (bar_item->associated_display > 0)
       && !(bar_item->associated_display & bar_mask)
       && (((bar_item->associated_bar << 1) & bar_mask)))
      || (bar_item->drawing
       && bar_item->associated_to_active_display
       && (((bar_item->associated_bar << 1) & bar_mask))
       && (bar->adid != bar_manager->active_adid));

    if (drawn_on_non_associated_display) return true;

    if (bar_item->type == BAR_COMPONENT_SPACE) continue;

    bool drawn_on_non_associated_space = bar_item->associated_space > 0
                                         && !(bar_item->associated_space
                                              & (1 << bar->sid))
                                         && ((bar_item->associated_bar << 1)
                                             & bar_mask);

    if (drawn_on_non_associated_space) return true;

    bool not_drawn_on_associated_space = draws_item
                                         && bar_item->associated_space > 0
                                         && (bar_item->associated_space
                                             & (1 << bar->sid))
                                         && !((bar_item->associated_bar << 1)
                                              & bar_mask);

    if (not_drawn_on_associated_space) return true;

  }
  return false;
}

void bar_manager_clear_needs_update(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) 
    bar_manager->bar_items[i]->needs_update = false;

  bar_manager->needs_ordering = false;
  bar_manager->bar_needs_update = false;
}

void bar_manager_reset_bar_association(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) 
    bar_item_reset_associated_bar(bar_manager->bar_items[i]);
}

void bar_manager_refresh(struct bar_manager* bar_manager, bool forced) {
  if (bar_manager->frozen) return;
  if (forced) {
    bar_manager_reset_bar_association(bar_manager);
    for (int j = 0; j < bar_manager->bar_item_count; j++) {
      bar_item_needs_update(bar_manager->bar_items[j]);
    }
  }

  if (forced || bar_manager->bar_needs_resize) bar_manager_resize(bar_manager);

  for (int i = 0; i < bar_manager->bar_count; ++i) {
    if (forced
        || bar_manager_bar_needs_redraw(bar_manager, bar_manager->bars[i])) { 
      bar_calculate_bounds(bar_manager->bars[i]);
      bar_draw(bar_manager->bars[i], false);
      if (bar_manager->needs_ordering) {
        bar_order_item_windows(bar_manager->bars[i]);
      }
    }
  }

  bar_manager_clear_needs_update(bar_manager);
}

void bar_manager_resize(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_resize(bar_manager->bars[i]);

  bar_manager->bar_needs_resize = false;
}

struct bar_item* bar_manager_create_item(struct bar_manager* bar_manager) {
  bar_manager->bar_items = (struct bar_item**) realloc(
                 bar_manager->bar_items,
                 sizeof(struct bar_item*) * (bar_manager->bar_item_count + 1));

  bar_manager->bar_item_count += 1;
  struct bar_item* bar_item = bar_item_create();
  bar_item_init(bar_item, &bar_manager->default_item);
  bar_manager->bar_items[bar_manager->bar_item_count - 1] = bar_item;
  bar_manager->needs_ordering = true;
  return bar_item;
}

void bar_manager_update_alias_components(struct bar_manager* bar_manager, bool forced) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    if ((!bar_item_is_shown(bar_manager->bar_items[i]) && !forced)
        || bar_manager->bar_items[i]->type != BAR_COMPONENT_ALIAS ) {
      continue;
    }

    alias_update(&bar_manager->bar_items[i]->alias, forced);
  }
}

void bar_manager_update_space_components(struct bar_manager* bar_manager, bool forced) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->type != BAR_COMPONENT_SPACE) continue;

    if (!bar_item->overrides_association) {
      uint32_t space = get_set_bit_position(bar_item->associated_space);
      uint32_t space_did = display_id_for_space(space);
      if (space_did) {
        bar_item->associated_display = 1 << (display_arrangement(space_did));
      }
      else {
        bar_item->associated_display = 1 << 30;
      }
    }
    for (int j = 0; j < bar_manager->bar_count; j++) {
      struct bar* bar = bar_manager->bars[j];
      uint32_t did = bar->adid;

      if ((1 << did) & bar_item->associated_display) {
        uint32_t sid = bar->sid;
        if (sid == 0) continue;
        if ((!bar_item->selected || forced)
            && bar_item->associated_space & (1 << sid)) {
          bar_item->selected = true;
          bar_item->updates = true;
          env_vars_set(&bar_item->signal_args.env_vars,
                       string_copy("SELECTED"),
                       string_copy("true")             );
        }
        else if ((bar_item->selected || forced)
                 && !(bar_item->associated_space & (1 << sid))) {
          bar_item->selected = false;
          bar_item->updates = true;
          env_vars_set(&bar_item->signal_args.env_vars,
                       string_copy("SELECTED"),
                       string_copy("false")            );
        }
        else {
          bar_item->updates = false;
        }
      } 
    }
  }
}

void bar_manager_animator_refresh(struct bar_manager* bar_manager) {
  bar_manager_freeze(bar_manager);
  if (animator_update(&bar_manager->animator)) {
    bar_manager_unfreeze(bar_manager);

    if (bar_manager->bar_needs_resize) bar_manager_resize(bar_manager);
    bar_manager_refresh(bar_manager, false);
  }
  bar_manager_unfreeze(bar_manager);
}

void bar_manager_update(struct bar_manager* bar_manager, bool forced) {
  if ((bar_manager->frozen && !forced) || bar_manager->sleeps) return;

  if (forced) {
    bar_manager_handle_space_change(bar_manager, true);
    forced_network_event();
    forced_volume_event();
    forced_brightness_event();
    forced_power_event();
    forced_front_app_event();
    forced_media_change_event();
    forced_space_windows_event();
  }

  bool needs_refresh = false;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    needs_refresh |= bar_item_update(bar_item, NULL, forced, NULL);

    if (bar_item->has_alias
        && bar_item_is_shown(bar_item)
        && alias_update(&bar_item->alias, false)) {
      bar_item_needs_update(bar_item);
      needs_refresh = true;
    }
  }

  if (needs_refresh || forced) bar_manager_refresh(bar_manager, forced);
}

void bar_manager_reset(struct bar_manager* bar_manager) {
  bar_manager_reset_bar_association(bar_manager);
  for (int i = 0; i < bar_manager->bar_count; i++) {
    for (int j = 0; j < bar_manager->bar_item_count; j++) {
      struct bar_item* bar_item = bar_manager->bar_items[j];
      bar_item_remove_window(bar_item, bar_manager->bars[i]->adid);
    }

    bar_destroy(bar_manager->bars[i]);
    bar_manager->bars[i] = NULL;
  }
  bar_manager->bar_count = 0;

  bar_manager_begin(bar_manager);
}

void bar_manager_begin(struct bar_manager* bar_manager) {
  if (bar_manager->displays == DISPLAY_MAIN_PATTERN) {
    uint32_t did = display_main_display_id();
    bar_manager->bar_count = 1;
    bar_manager->bars = (struct bar **) realloc(
                                bar_manager->bars,
                                sizeof(struct bar *) * bar_manager->bar_count);

    memset(bar_manager->bars, 0, sizeof(struct bar*) * bar_manager->bar_count);
    bar_manager->bars[0] = bar_create(did);
    bar_manager->bars[0]->adid = display_arrangement(did);
  } 
  else {
    uint32_t display_count = display_active_display_count();
    uint32_t bar_count = 0;
    for (uint32_t index = 1; index <= display_count; index++) {
      if (!(bar_manager->displays & 1 << (index - 1))) continue;
      bar_count++;
    }

    bar_manager->bar_count = bar_count;
    bar_manager->bars = (struct bar **) realloc(
                                bar_manager->bars,
                                sizeof(struct bar *) * bar_manager->bar_count);

    memset(bar_manager->bars, 0, sizeof(struct bar*) * bar_manager->bar_count);

    uint32_t bar_index = 0;
    for (uint32_t index = 1; index <= display_count; index++) {
      if (!(bar_manager->displays & 1 << (index - 1))) continue;
      uint32_t did = display_arrangement_display_id(index);
      bar_manager->bars[bar_index] = bar_create(did);
      bar_manager->bars[bar_index]->adid = index;
      if (bar_manager->any_bar_hidden)
        bar_set_hidden(bar_manager->bars[bar_index], true);

      bar_index++;
    }
  }

  bar_manager->active_displays = 0;
  for (int i = 0; i < bar_manager->bar_count; i++) {
    bar_manager->active_displays |= 1 << bar_manager->bars[i]->adid;
  }

  bar_manager->active_adid = display_active_display_adid();
  bar_manager->needs_ordering = true;
}

struct bar_item* bar_manager_get_item_by_point(struct bar_manager* bar_manager, CGPoint point, struct window** window_out) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (!bar_item->drawing) continue;

    for (int adid = 1; adid <= bar_item->num_windows; adid++) {
      struct window* window = bar_item_get_window(bar_item, adid);
      if (!window) continue;

      CGRect frame = window->frame;
      frame.origin = window->origin;
      if (cgrect_contains_point(&frame, &point)) {
        if (window_out) *window_out = window;
        return bar_item;
      }
    }
  }
  return NULL;
}

struct bar_item* bar_manager_get_item_by_wid(struct bar_manager* bar_manager, uint32_t wid, struct window** window_out) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (!bar_item->drawing) continue;

    for (int adid = 1; adid <= bar_item->num_windows; adid++) {
      struct window* window = bar_item_get_window(bar_item, adid);
      if (!window) continue;

      if (window->id == wid) {
        if (window_out) *window_out = window;
        return bar_item;
      }
    }
  }
  return NULL;
}

struct bar* bar_manager_get_bar_by_wid(struct bar_manager* bar_manager, uint32_t wid) {
  for (int i = 0; i < bar_manager->bar_count; i++) {
    if (bar_manager->bars[i]->window.id == wid) {
      return bar_manager->bars[i];
    }
  }
  return NULL;
}

bool bar_manager_mouse_over_any_bar(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_count; i++) {
    if (bar_manager->bars[i]->mouse_over) return true;
  }
  return false;
}

struct popup* bar_manager_get_popup_by_wid(struct bar_manager* bar_manager, uint32_t wid) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (!bar_item->drawing || !bar_item->popup.drawing) {
      continue;
    }

    struct window* window = &bar_item->popup.window;

    if (window->id == wid) {
      return &bar_item->popup;
    }
  }
  return NULL;
}

struct popup* bar_manager_get_popup_by_point(struct bar_manager* bar_manager, CGPoint point) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (!bar_item->drawing || !bar_item->popup.drawing) {
      continue;
    }

    struct window* window = &bar_item->popup.window;

    CGRect frame = window->frame;
    frame.origin = window->origin;

    if (CGRectContainsPoint(frame, point)) return &bar_item->popup;
  }
  return NULL;

}

struct bar* bar_manager_get_bar_by_point(struct bar_manager* bar_manager, CGPoint point) {
  for (int i = 0; i < bar_manager->bar_count; i++) {
    struct window* window = &bar_manager->bars[i]->window;

    CGRect frame = window->frame;
    frame.origin = window->origin;

    if (CGRectContainsPoint(frame, point)) return bar_manager->bars[i];
  }
  return NULL;
}

bool bar_manager_mouse_over_any_popup(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (!bar_item->drawing || !bar_item->popup.drawing) {
      continue;
    }
    if (bar_item->popup.mouse_over) return true;
  }
  return false;
}

void bar_manager_custom_events_trigger(struct bar_manager* bar_manager, char* name, struct env_vars* env_vars) {
  uint64_t flag = custom_events_get_flag_for_name(&bar_manager->custom_events,
                                                  name                       );

  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->update_mask & flag)
      bar_item_update(bar_item, name, false, env_vars);
  }
}

void bar_manager_display_resized(struct bar_manager* bar_manager, uint32_t did) {
  for (int i = 0; i < bar_manager->bar_count; i++) {
    if (bar_manager->bars[i]->did == did) {
      bar_resize(bar_manager->bars[i]);
    }
  }
  bar_manager_refresh(bar_manager, false);
}

void bar_manager_display_moved(struct bar_manager* bar_manager, uint32_t did) {
  for (int i = 0; i < bar_manager->bar_count; i++) {
    struct bar* bar = bar_manager->bars[i];
    if (bar->did == did) {
      bar_resize(bar);
      bar->adid = display_arrangement(bar->did);
      bar->dsid = display_space_id(bar->did);
      bar->sid = mission_control_index(bar->dsid);
    }
  }
  bar_manager->needs_ordering = true;
  bar_manager_refresh(bar_manager, true);
  bar_manager_handle_display_change(bar_manager);
}

void bar_manager_display_removed(struct bar_manager* bar_manager, uint32_t did) {
  bar_manager_display_changed(bar_manager);
}

void bar_manager_display_added(struct bar_manager* bar_manager, uint32_t did) {
  bar_manager_display_changed(bar_manager);
}

void bar_manager_display_changed(struct bar_manager* bar_manager) {
  bar_manager->active_adid = display_active_display_adid();

  bar_manager_freeze(bar_manager);
  bar_manager_reset(bar_manager);
  bar_manager_unfreeze(bar_manager);
  bar_manager_refresh(bar_manager, true);

  bar_manager_handle_display_change(bar_manager);
  bar_manager_handle_space_change(bar_manager, true);
  animator_renew_display_link(&bar_manager->animator);
}

void bar_manager_cancel_drag(struct bar_manager* bar_manager) {
  for (uint32_t i = 0; i < bar_manager->bar_item_count; i++) {
    bar_item_cancel_drag(bar_manager->bar_items[i]);
  }
}

void bar_manager_handle_mouse_entered_global(struct bar_manager* bar_manager) {
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_MOUSE_ENTERED_GLOBAL,
                                    NULL                                   );
}

void bar_manager_handle_mouse_exited_global(struct bar_manager* bar_manager) {
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_MOUSE_EXITED_GLOBAL,
                                    NULL                                  );

  bar_manager_handle_mouse_exited(bar_manager, NULL);
}

void bar_manager_handle_mouse_scrolled_global(struct bar_manager* bar_manager, int scroll_delta, uint32_t adid) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  char delta_ver_str[32];
  snprintf(delta_ver_str, 32, "%d", scroll_delta);
  env_vars_set(&env_vars,
               string_copy("SCROLL_DELTA"),
               string_copy(delta_ver_str));

  char info_str[256];
  snprintf(info_str, 256, "{\n"
                          "\t\"delta\": %d\n"
                          "}\n",
                          scroll_delta       );

  env_vars_set(&env_vars, string_copy("INFO"), string_copy(info_str));

  char adid_str[32];
  snprintf(adid_str, 32, "%u", adid);
  env_vars_set(&env_vars,
               string_copy("DID"),
               string_copy(adid_str));

  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_MOUSE_SCROLLED_GLOBAL,
                                    &env_vars                               );
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_mouse_entered(struct bar_manager* bar_manager, struct bar_item* bar_item) {
  if (!bar_item) return;
  bar_item_mouse_entered(bar_item);
}

void bar_manager_handle_mouse_exited(struct bar_manager* bar_manager, struct bar_item* bar_item) {
  if (!bar_item) {
    for (int i = 0; i < bar_manager->bar_item_count; i++)
      bar_item_mouse_exited(bar_manager->bar_items[i]);
  } else {
    bar_item_mouse_exited(bar_item);
  }
}

void bar_manager_handle_volume_change(struct bar_manager* bar_manager, float volume) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  char volume_str[16];
  snprintf(volume_str, 16, "%d", (int)(volume*100. + 0.5));
  env_vars_set(&env_vars, string_copy("INFO"), string_copy(volume_str));
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_VOLUME_CHANGE,
                                    &env_vars                       );
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_wifi_change(struct bar_manager* bar_manager, char* ssid) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  env_vars_set(&env_vars, string_copy("INFO"), string_copy(ssid));
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_WIFI_CHANGE,
                                    &env_vars                     );
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_brightness_change(struct bar_manager* bar_manager, float brightness) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  char brightness_str[16];
  snprintf(brightness_str, 16, "%d", (int)(brightness*100. + 0.5));
  env_vars_set(&env_vars, string_copy("INFO"), string_copy(brightness_str));
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_BRIGHTNESS_CHANGE,
                                    &env_vars                           );
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_power_source_change(struct bar_manager* bar_manager, char* state) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  env_vars_set(&env_vars, string_copy("INFO"), string_copy(state));
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_POWER_SOURCE_CHANGE,
                                    &env_vars                             );
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_media_change(struct bar_manager* bar_manager, char* info) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  env_vars_set(&env_vars, string_copy("INFO"), string_copy(info));
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_MEDIA_CHANGE,
                                    &env_vars                      );
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_media_cover_change(struct bar_manager* bar_manager, CGImageRef image) {
  bool needs_refresh = false;
  if (image_set_image(&bar_manager->current_artwork, image, CGRectNull, false)){
    for (int i = 0; i < bar_manager->bar_item_count; i++) {
      struct bar_item* bar_item = bar_manager->bar_items[i];
      needs_refresh |= bar_item_set_media_cover(bar_item,
                                                &bar_manager->current_artwork);
    }
  }

  if (needs_refresh) bar_manager_refresh(bar_manager, false);
}

void bar_manager_handle_front_app_switch(struct bar_manager* bar_manager, char* info) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  if (info) env_vars_set(&env_vars, string_copy("INFO"), info);
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_FRONT_APP_SWITCHED,
                                    &env_vars                            );
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_space_windows_change(struct bar_manager* bar_manager, char* info) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  if (info) env_vars_set(&env_vars, string_copy("INFO"), string_copy(info));
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_SPACE_WINDOWS_CHANGE,
                                    &env_vars                              );
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_space_change(struct bar_manager* bar_manager, bool forced) {
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  char info[19 * bar_manager->bar_count + 4];
  info[0] = '{';
  info[1] = '\n';
  uint32_t cursor = 2;
  char separator[] = ",";
  bar_manager_freeze(bar_manager);
  bool force_refresh = false;
  for (int i = 0; i < bar_manager->bar_count; i++) {
    uint64_t dsid = display_space_id(bar_manager->bars[i]->did);
    bar_manager->bars[i]->sid = mission_control_index(dsid);

    bool was_shown = bar_manager->bars[i]->shown;
    bar_manager->bars[i]->shown = SLSSpaceGetType(g_connection, dsid) != 4;

    bar_manager->needs_ordering |= !was_shown && bar_manager->bars[i]->shown;
    force_refresh |= !was_shown && bar_manager->bars[i]->shown;
    force_refresh |= !bar_manager->bars[i]->shown && was_shown;

    if (bar_manager->bars[i]->dsid != dsid) {
      bar_manager->bars[i]->dsid = dsid;
      if (!bar_manager->sticky && bar_manager->bars[i]->shown)
        bar_change_space(bar_manager->bars[i], dsid);
    }
    if (i == bar_manager->bar_count - 1)
      separator[0] = '\0';

    snprintf(info + cursor, 19 * bar_manager->bar_count + 4 - cursor,
                            "\t\"display-%d\": %d%s\n",
                            bar_manager->bars[i]->adid,
                            bar_manager->bars[i]->sid,
                            separator                                );
    cursor = strlen(info);
  }

  info[cursor] = '}';
  info[cursor + 1] = '\0';
  env_vars_set(&env_vars, string_copy("INFO"), string_copy(info));

  bar_manager_update_space_components(bar_manager, forced);
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_SPACE_CHANGE,
                                    &env_vars                      );


  bar_manager_unfreeze(bar_manager);
  bar_manager_refresh(bar_manager, force_refresh);
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_display_change(struct bar_manager* bar_manager) {
  bar_manager->active_adid = display_active_display_adid();
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  char adid_str[3];
  snprintf(adid_str, 3, "%d", bar_manager->active_adid);
  env_vars_set(&env_vars, string_copy("INFO"), string_copy(adid_str));

  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_DISPLAY_CHANGE,
                                    &env_vars                        );

  bar_manager_refresh(bar_manager, false);
  env_vars_destroy(&env_vars);
}

void bar_manager_handle_system_will_sleep(struct bar_manager* bar_manager) {
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_SYSTEM_WILL_SLEEP,
                                    NULL                                );
  animator_destroy_display_link(&bar_manager->animator);
  bar_manager->sleeps = true;
}

void bar_manager_handle_system_woke(struct bar_manager* bar_manager) {
  usleep(100000);
  bar_manager->sleeps = false;
  bar_manager_custom_events_trigger(bar_manager,
                                    COMMAND_SUBSCRIBE_SYSTEM_WOKE,
                                    NULL                          );


  bar_manager_display_changed(bar_manager);
}

void bar_manager_handle_notification(struct bar_manager* bar_manager, struct notification* notification) {
  char* name = custom_events_get_name_for_notification(&bar_manager->custom_events, notification->name);
  if (!name) {
    notification_destroy(notification);
    return;
  }

  struct env_vars env_vars;
  env_vars_init(&env_vars);
  if (notification->info) env_vars_set(&env_vars,
                                       string_copy("INFO"),
                                       string_copy(notification->info));

  bar_manager_custom_events_trigger(bar_manager, name, &env_vars);
  env_vars_destroy(&env_vars);
  notification_destroy(notification);
}

void bar_manager_destroy(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->event_port) {
      mach_send_message(bar_item->event_port, "k", 2, false);
    }
  }

  animator_destroy(&bar_manager->animator);

  while (bar_manager->bar_item_count > 0) {
    bar_manager_remove_item(bar_manager, bar_manager->bar_items[0]);
  }

  if (bar_manager->bar_items) free(bar_manager->bar_items);
  for (int i = 0; i < bar_manager->bar_count; i++) {
    bar_destroy(bar_manager->bars[i]);
  }

  bar_item_destroy(&bar_manager->default_item, false);
  custom_events_destroy(&bar_manager->custom_events);
  background_destroy(&bar_manager->background);

  if (bar_manager->bars) free(bar_manager->bars);
  CFRunLoopRemoveTimer(CFRunLoopGetMain(),
                       bar_manager->clock,
                       kCFRunLoopCommonModes);

  CFRunLoopTimerInvalidate(bar_manager->clock);
  CFRelease(bar_manager->clock);
  image_destroy(&bar_manager->current_artwork);
}

void bar_manager_serialize(struct bar_manager* bar_manager, FILE* rsp) {
  char indent[] = { "\t" };
  fprintf(rsp, "{\n"
               "%s\"position\": \"%s\",\n"
               "%s\"topmost\": \"%s\",\n"
               "%s\"sticky\": \"%s\",\n"
               "%s\"hidden\": \"%s\",\n"
               "%s\"shadow\": \"%s\",\n"
               "%s\"font_smoothing\": \"%s\",\n"
               "%s\"blur_radius\": %u,\n"
               "%s\"margin\": %d,\n",
               indent, bar_manager->position == POSITION_BOTTOM
                                              ? "bottom" : "top",
               indent, format_bool(bar_manager->topmost),
               indent, format_bool(bar_manager->sticky),
               indent, format_bool(bar_manager->any_bar_hidden),
               indent, format_bool(bar_manager->shadow),
               indent, format_bool(bar_manager->font_smoothing),
               indent, bar_manager->blur_radius,
               indent, bar_manager->margin         );

  background_serialize(&bar_manager->background, indent, rsp, false);

  fprintf(rsp, ",\n%s\"items\": [\n", indent);
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    fprintf(rsp, "%s\t \"%s\"", indent, bar_manager->bar_items[i]->name);
    if (i < bar_manager->bar_item_count - 1) fprintf(rsp, ",\n");
  }
  fprintf(rsp, "\n%s]\n}\n", indent);
}
