#pragma once
#include "bar.h"
#include "bar_item.h"
#include "animation.h"

#define CLOCK_CALLBACK(name) void name(CFRunLoopTimerRef timer, void *context)
typedef CLOCK_CALLBACK(clock_callback);

#define DISPLAY_MAIN_PATTERN 0
#define DISPLAY_ALL_PATTERN  UINT32_MAX

#define TOPMOST_LEVEL_WINDOW 'w'
#define TOPMOST_LEVEL_ALL    'a'

struct bar_manager {
  CFRunLoopTimerRef clock;

  bool frozen;
  bool sleeps;
  bool shadow;
  bool topmost;
  bool sticky;
  bool font_smoothing;
  bool any_bar_hidden;
  bool needs_ordering;
  bool might_need_clipping;
  bool bar_needs_update;
  bool bar_needs_resize;

  uint32_t displays;
  char position;

  int margin;
  uint32_t blur_radius;
  uint32_t notch_width;
  uint32_t notch_offset;
  uint32_t active_adid;
  uint32_t window_level;

  struct bar** bars;
  uint32_t bar_count;
  uint32_t active_displays;

  struct bar_item** bar_items;
  struct bar_item default_item;
  uint32_t bar_item_count;

  struct background background;
  struct custom_events custom_events;

  struct animator animator;
  struct image current_artwork;
};

void bar_manager_init(struct bar_manager* bar_manager);
void bar_manager_begin(struct bar_manager* bar_manager);
void bar_manager_reset(struct bar_manager* bar_manager);

struct bar_item* bar_manager_create_item(struct bar_manager* bar_manager);
void bar_manager_remove_item(struct bar_manager* bar_manager, struct bar_item* bar_item);
void bar_manager_move_item(struct bar_manager* bar_manager, struct bar_item* item, struct bar_item* reference, bool before);
void bar_manager_handle_notification(struct bar_manager* bar_manager, struct notification* notification);

void bar_manager_animator_refresh(struct bar_manager* bar_manager);
void bar_manager_update(struct bar_manager* bar_manager, bool forced);
void bar_manager_update_space_components(struct bar_manager* bar_manager, bool forced);
bool bar_manager_set_margin(struct bar_manager* bar_manager, int margin);
bool bar_manager_set_y_offset(struct bar_manager* bar_manager, int y_offset);
bool bar_manager_set_bar_height(struct bar_manager* bar_manager, int height);
bool bar_manager_set_background_blur(struct bar_manager* bar_manager, uint32_t radius);
bool bar_manager_set_position(struct bar_manager* bar_manager, char pos);
bool bar_manager_set_spaces(struct bar_manager* bar_manager, bool value);
bool bar_manager_set_spaces_for_all_displays(struct bar_manager* bar_manager, bool value);
bool bar_manager_set_displays(struct bar_manager* bar_manager, uint32_t displays);
bool bar_manager_set_hidden(struct bar_manager* bar_manager, uint32_t sid, bool hidden);
bool bar_manager_set_topmost(struct bar_manager* bar_manager, char level, bool topmost);
bool bar_manager_set_sticky(struct bar_manager *bar_manager, bool sticky);
bool bar_manager_set_shadow(struct bar_manager* bar_manager, bool shadow);
bool bar_manager_set_font_smoothing(struct bar_manager* bar_manager, bool smoothing);
bool bar_manager_set_notch_width(struct bar_manager* bar_manager, uint32_t width);
bool bar_manager_set_notch_offset(struct bar_manager* bar_manager, uint32_t offset);
void bar_manager_sort(struct bar_manager* bar_manager, struct bar_item** ordering, uint32_t count);

struct bar_item* bar_manager_get_item_by_point(struct bar_manager* bar_manager, CGPoint point, struct window** window_out);
struct bar* bar_manager_get_bar_by_point(struct bar_manager* bar_manager, CGPoint point);
struct popup* bar_manager_get_popup_by_point(struct bar_manager* bar_manager, CGPoint point);
struct bar_item* bar_manager_get_item_by_wid(struct bar_manager* bar_manager, uint32_t wid, struct window** window_out);
struct popup* bar_manager_get_popup_by_wid(struct bar_manager* bar_manager, uint32_t wid);
struct bar* bar_manager_get_bar_by_wid(struct bar_manager* bar_manager, uint32_t wid);
int bar_manager_get_item_index_for_name(struct bar_manager* bar_manager, char* name);
uint32_t bar_manager_length_for_bar_side(struct bar_manager* bar_manager, struct bar* bar, char side);
bool bar_manager_mouse_over_any_popup(struct bar_manager* bar_manager);
bool bar_manager_mouse_over_any_bar(struct bar_manager* bar_manager);

void bar_manager_freeze(struct bar_manager* bar_manager);
void bar_manager_unfreeze(struct bar_manager* bar_manager);

void bar_manager_display_changed(struct bar_manager* bar_manager);
void bar_manager_display_resized(struct bar_manager* bar_manager, uint32_t did);
void bar_manager_display_moved(struct bar_manager* bar_manager, uint32_t did);
void bar_manager_display_removed(struct bar_manager* bar_manager, uint32_t did);
void bar_manager_display_added(struct bar_manager* bar_manager, uint32_t did);
void bar_manager_refresh(struct bar_manager* bar_manager, bool forced);
void bar_manager_resize(struct bar_manager* bar_manager);

void bar_manager_handle_mouse_entered_global(struct bar_manager* bar_manager);
void bar_manager_handle_mouse_exited_global(struct bar_manager* bar_manager);
void bar_manager_handle_mouse_scrolled_global(struct bar_manager* bar_manager, int scroll_delta, uint32_t did);
void bar_manager_handle_mouse_entered(struct bar_manager* bar_manager, struct bar_item* bar_item);
void bar_manager_handle_mouse_exited(struct bar_manager* bar_manager, struct bar_item* bar_item);
void bar_manager_handle_front_app_switch(struct bar_manager* bar_manager, char* info);
void bar_manager_handle_space_change(struct bar_manager* bar_manager, bool forced);
void bar_manager_handle_display_change(struct bar_manager* bar_manager);
void bar_manager_handle_system_woke(struct bar_manager* bar_manager);
void bar_manager_handle_system_will_sleep(struct bar_manager* bar_manager);
void bar_manager_handle_volume_change(struct bar_manager* bar_manager, float volume);
void bar_manager_handle_wifi_change(struct bar_manager* bar_manager, char* ssid);
void bar_manager_handle_brightness_change(struct bar_manager* bar_manager, float brightness);
void bar_manager_handle_power_source_change(struct bar_manager* bar_manager, char* state);
void bar_manager_handle_media_change(struct bar_manager* bar_manager, char* info);
void bar_manager_handle_media_cover_change(struct bar_manager* bar_manager, CGImageRef image);
void bar_manager_handle_space_windows_change(struct bar_manager* bar_manager, char* info);
void bar_manager_custom_events_trigger(struct bar_manager* bar_manager, char* name, struct env_vars* env_vars);

void bar_manager_destroy(struct bar_manager* bar_manager);

void bar_manager_serialize(struct bar_manager* bar_manager, FILE* rsp);
