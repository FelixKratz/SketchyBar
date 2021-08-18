#include "bar_manager.h"

extern struct event_loop g_event_loop;

static TIMER_CALLBACK(timer_handler)
{
  struct event *event = event_create(&g_event_loop, BAR_REFRESH, NULL);
  event_loop_post(&g_event_loop, event);
}

static SHELL_TIMER_CALLBACK(shell_timer_handler)
{
  struct event *event = event_create(&g_event_loop, SHELL_REFRESH, NULL);
  event_loop_post(&g_event_loop, event);
}

int bar_manager_get_item_index_for_name(struct bar_manager* bar_manager, char* name) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    if (strcmp(bar_manager->bar_items[i]->name, name) == 0) {
      return i;
    }
  }
  return -1;
}

void bar_manager_set_background_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->background_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_position(struct bar_manager *bar_manager, char *pos)
{
  bar_manager->position = pos;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_height(struct bar_manager *bar_manager, uint32_t height)
{
  bar_manager->height = height;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_padding_left(struct bar_manager *bar_manager, uint32_t padding)
{
  bar_manager->padding_left = padding;
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_padding_right(struct bar_manager *bar_manager, uint32_t padding)
{
  bar_manager->padding_right = padding;
  bar_manager_refresh(bar_manager);
}

void bar_manager_display_changed(struct bar_manager *bar_manager)
{
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_destroy(bar_manager->bars[i]);

  bar_manager_begin(bar_manager);
}

void bar_manager_set_display(struct bar_manager *bar_manager, char *display)
{
  bar_manager->display = display;

  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_destroy(bar_manager->bars[i]);

  bar_manager_begin(bar_manager);
}

void bar_manager_refresh(struct bar_manager *bar_manager)
{
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_refresh(bar_manager->bars[i]);
}

void bar_manager_resize(struct bar_manager *bar_manager)
{
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_resize(bar_manager->bars[i]);
}

struct bar_item* bar_manager_create_item(struct bar_manager* bar_manager) {
    bar_manager->bar_items = (struct bar_item**) realloc(bar_manager->bar_items, sizeof(struct bar_item*) * (bar_manager->bar_item_count + 1));
    bar_manager->bar_item_count += 1;
    struct bar_item* bar_item = bar_item_create();
    bar_item_init(bar_item);
    bar_manager->bar_items[bar_manager->bar_item_count - 1] = bar_item;
    return bar_item;
}

void bar_manager_init(struct bar_manager *bar_manager)
{
  bar_manager->bars = NULL;
  bar_manager->bar_count = 0;
  bar_manager->bar_item_count = 0;
  bar_manager->display = BAR_DISPLAY_ALL;
  bar_manager->position = BAR_POSITION_TOP;
  bar_manager->height = 25;
  bar_manager->padding_left = 20;
  bar_manager->padding_right = 20;
  
  int refresh_frequency = 1;
  int shell_refresh_frequency = 1;

  bar_manager->refresh_timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + refresh_frequency, refresh_frequency, 0, 0, timer_handler, NULL);
  CFRunLoopAddTimer(CFRunLoopGetMain(), bar_manager->refresh_timer, kCFRunLoopCommonModes);
  
  bar_manager->shell_refresh_timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + shell_refresh_frequency, shell_refresh_frequency, 0, 0, shell_timer_handler, NULL);
  CFRunLoopAddTimer(CFRunLoopGetMain(), bar_manager->shell_refresh_timer, kCFRunLoopCommonModes);
}

void bar_manager_update_components(struct bar_manager* bar_manager, uint32_t did, uint32_t sid) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    bar_item_update_component(bar_item, did, sid);
  }
}

void bar_manager_script_update(struct bar_manager* bar_manager, bool forced) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    bar_item_script_update(bar_manager->bar_items[i], forced);
  }
}

void bar_manager_begin(struct bar_manager *bar_manager)
{
  if (strcmp(bar_manager->display, BAR_DISPLAY_MAIN_ONLY) == 0) {
    uint32_t did = display_manager_main_display_id();
    bar_manager->bar_count = 1;
    bar_manager->bars = (struct bar **) malloc(sizeof(struct bar *) * bar_manager->bar_count);
    memset(bar_manager->bars,0, sizeof(struct bar*) * bar_manager->bar_count);
    bar_manager->bars[0] = bar_create(did);
  } 
  else if (strcmp(bar_manager->display, BAR_DISPLAY_ALL) == 0) {
    bar_manager->bar_count = display_manager_active_display_count();
    bar_manager->bars = (struct bar **) malloc(sizeof(struct bar *) * bar_manager->bar_count);
    memset(bar_manager->bars,0, sizeof(struct bar*) * bar_manager->bar_count);
    for (uint32_t index=1; index <= bar_manager->bar_count; index++) {
      uint32_t did = display_manager_arrangement_display_id(index);
      bar_manager->bars[index - 1] = bar_create(did);
    }
  }
  else return;
}

void bar_manager_check_bar_items_for_update_pattern(struct bar_manager* bar_manager, uint32_t pattern) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->update_mask & pattern)
      bar_item_script_update(bar_item, true);
  }
}

struct bar_item* bar_manager_get_item_by_point(struct bar_manager* bar_manager, CGPoint point, uint32_t sid) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->num_rects < sid || bar_item->bounding_rects[sid - 1] == NULL) continue;
    if (cgrect_contains_point(bar_item->bounding_rects[sid - 1], &point)) {
      return bar_item;
    }
  }
  return NULL;
}

void bar_manager_handle_front_app_switch(struct bar_manager* bar_manager) {
  bar_manager_check_bar_items_for_update_pattern(bar_manager, UPDATE_FRONT_APP_SWITCHED);
}

void bar_manager_handle_window_focus(struct bar_manager* bar_manager) {
  bar_manager_check_bar_items_for_update_pattern(bar_manager, UPDATE_WINDOW_FOCUS);
}

void bar_manager_handle_title_change(struct bar_manager* bar_manager) {
  bar_manager_check_bar_items_for_update_pattern(bar_manager, UPDATE_TITLE_CHANGE);
}

void bar_manager_handle_space_change(struct bar_manager* bar_manager) {
  bar_manager_check_bar_items_for_update_pattern(bar_manager, UPDATE_SPACE_CHANGE);
}

void bar_manager_handle_display_change(struct bar_manager* bar_manager) {
  bar_manager_check_bar_items_for_update_pattern(bar_manager, UPDATE_DISPLAY_CHANGE);
}

void bar_manager_handle_system_woke(struct bar_manager* bar_manager) {
  bar_manager_check_bar_items_for_update_pattern(bar_manager, UPDATE_SYSTEM_WOKE);
}

