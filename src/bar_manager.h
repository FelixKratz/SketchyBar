#ifndef BAR_MANAGER_H
#define BAR_MANAGER_H
#include <stdint.h>
#define TIMER_CALLBACK(name) void name(CFRunLoopTimerRef timer, void *context)
typedef TIMER_CALLBACK(timer_callback);

#define SHELL_TIMER_CALLBACK(name) void name(CFRunLoopTimerRef timer, void *context)
typedef SHELL_TIMER_CALLBACK(shell_timer_callback);


struct bar_manager {
  bool frozen;
  bool hidden;
  bool topmost;
  uint32_t window_level;
  CFRunLoopTimerRef refresh_timer;
  CFRunLoopTimerRef shell_refresh_timer;
  struct bar **bars;
  int bar_count;
  struct bar_item **bar_items;
  struct bar_item default_item;
  int bar_item_count;
  char *position;
  char *display;
  uint32_t height;
  uint32_t margin;
  uint32_t corner_radius;
  uint32_t blur_radius;
  uint32_t y_offset;
  uint32_t padding_left;
  uint32_t padding_right;

  uint32_t border_width;
  struct rgba_color background_color;
  struct rgba_color border_color;
  struct custom_events custom_events;
};

int bar_manager_get_item_index_for_name(struct bar_manager* bar_manager, char* name);
void bar_manager_custom_events_trigger(struct bar_manager* bar_manager, char* name);


struct bar_item* bar_manager_create_item(struct bar_manager* bar_manager);
void bar_manager_destroy_item(struct bar_manager* bar_manager, struct bar_item* bar_item);
void bar_manager_handle_notification(struct bar_manager* bar_manager, char* context);

void bar_manager_script_update(struct bar_manager* bar_manager, bool forced);
void bar_manager_update_components(struct bar_manager* bar_manager, bool forced);
void bar_manager_set_background_blur(struct bar_manager* bar_manager, uint32_t radius);
void bar_manager_set_background_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_position(struct bar_manager *bar_manager, char *pos);
void bar_manager_set_spaces(struct bar_manager *bar_manager, bool value);
void bar_manager_set_spaces_for_all_displays(struct bar_manager *bar_manager, bool value);
void bar_manager_set_height(struct bar_manager *bar_manager, uint32_t height);
void bar_manager_set_padding_left(struct bar_manager *bar_manager, uint32_t padding);
void bar_manager_set_padding_right(struct bar_manager *bar_manager, uint32_t padding);
void bar_manager_set_display(struct bar_manager *bar_manager, char *display);
void bar_manager_set_hidden(struct bar_manager *bar_manager, bool hidden);
void bar_manager_set_topmost(struct bar_manager *bar_manager, bool topmost);
void bar_manager_set_border_width(struct bar_manager* bar_manager, uint32_t width);
void bar_manager_set_border_color(struct bar_manager* bar_manager, uint32_t color);
void bar_manager_freeze(struct bar_manager *bar_manager);
void bar_manager_unfreeze(struct bar_manager *bar_manager);

void bar_manager_display_changed(struct bar_manager *bar_manager);
void bar_manager_refresh(struct bar_manager *bar_manager, bool forced);
void bar_manager_resize(struct bar_manager *bar_manager);
void bar_manager_begin(struct bar_manager *bar_manager);
void bar_manager_init(struct bar_manager *bar_manager);

void bar_manager_handle_front_app_switch(struct bar_manager* bar_manager);
void bar_manager_handle_space_change(struct bar_manager* bar_manager);
void bar_manager_handle_display_change(struct bar_manager* bar_manager);
void bar_manager_handle_system_woke(struct bar_manager* bar_manager);

struct bar_item* bar_manager_get_item_by_point(struct bar_manager* bar_manager, CGPoint point, uint32_t sid);

#endif
