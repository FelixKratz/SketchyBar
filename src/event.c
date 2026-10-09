#include "event.h"
#include "bar_manager.h"
#include "custom_events.h"
#include "hotload.h"

extern struct bar_manager g_bar_manager;
extern int g_connection;
extern int g_space_management_mode;

static void event_distributed_notification(void* context) {
  bar_manager_handle_notification(&g_bar_manager, context);
}

static void event_application_front_switched(void* context) {
  bar_manager_handle_front_app_switch(&g_bar_manager, context);
}

static void event_space_changed(void* context) {
  bar_manager_handle_space_change(&g_bar_manager, false);
}

static void event_display_changed(void* context) {
  bar_manager_handle_display_change(&g_bar_manager);
}

static void event_display_added(void* context) {
  uint32_t did = (uint32_t)(intptr_t)context;
  bar_manager_display_added(&g_bar_manager, did);
}

static void event_display_removed(void* context) {
  uint32_t did = (uint32_t)(intptr_t)context;
  bar_manager_display_removed(&g_bar_manager, did);
}

static void event_display_moved(void* context) {
  uint32_t did = (uint32_t)(intptr_t)context;
  bar_manager_display_moved(&g_bar_manager, did);
}

static void event_display_resized(void* context) {
  uint32_t did = (uint32_t)(intptr_t)context;
  bar_manager_display_resized(&g_bar_manager, did);
}

static void event_menu_bar_hidden_changed(void* context) {
  bar_manager_resize(&g_bar_manager);
  g_bar_manager.bar_needs_update = true;
  bar_manager_refresh(&g_bar_manager, false);
}

static void event_system_woke(void* context) {
  bar_manager_handle_system_woke(&g_bar_manager);
}

static void event_system_will_sleep(void* context) {
  bar_manager_handle_system_will_sleep(&g_bar_manager);
}

static void event_shell_refresh(void* context) {
  bar_manager_update(&g_bar_manager, false);
}

static void event_animator_refresh(void* context) {
  bar_manager_animator_refresh(&g_bar_manager, (uint64_t)context);
}

static void event_mach_message(void* context) {
  handle_message_mach(context);
}

static void event_mouse_up(void* context) {
  CGPoint point = CGEventGetLocation(context);
  uint32_t wid = get_wid_from_cg_event(context);
  CGEventType type = CGEventGetType(context);
  uint32_t mouse_button_code = CGEventGetIntegerValueField(context, kCGMouseEventButtonNumber);
  uint32_t modifier_keys = CGEventGetFlags(context);

  struct window* window = NULL;
  struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                          wid,
                                                          &window        );

  if (!bar_item || bar_item->type == BAR_COMPONENT_GROUP) {
    bar_item = bar_manager_get_item_by_point(&g_bar_manager, point, &window);
  }

  struct bar* bar = bar_manager_get_bar_by_wid(&g_bar_manager, wid);
  struct popup* popup = bar_manager_get_popup_by_wid(&g_bar_manager, wid);
  if (!bar_item && !popup && !bar) return;

  CGPoint point_in_window_coords = CGPointZero;
  if (bar_item && window) {
    point_in_window_coords.x = point.x - window->origin.x;
    point_in_window_coords.y = point.y - window->origin.y;
  }

  bar_item_on_click(bar_item,
                    type,
                    mouse_button_code,
                    modifier_keys,
                    point_in_window_coords);

  if (bar_item && bar_item->needs_update)
    bar_manager_refresh(&g_bar_manager, false);
}

static void event_mouse_dragged(void* context) {
  CGPoint point = CGEventGetLocation(context);
  uint32_t wid = get_wid_from_cg_event(context);

  struct window* window = NULL;
  struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                          wid,
                                                          &window        );

  if (!bar_item || !bar_item->has_slider) return;

  CGPoint point_in_window_coords = CGPointZero;
  if (bar_item && window) {
    point_in_window_coords.x = point.x - window->origin.x;
    point_in_window_coords.y = point.y - window->origin.y;
  }

  bar_item_on_drag(bar_item, point_in_window_coords);

  if (bar_item->needs_update)
    bar_manager_refresh(&g_bar_manager, false);
}

// Exit events cannot be judged from geometry alone. Their location can lag
// behind the pointer, and another application's window can overlap the edge
// of the bar (e.g. the resize border of a window placed right below it), so a
// point inside the bar frame can already be over a foreign window. Ask the
// window server which window is under the cursor right now, and resolve the
// bar or popup it belongs to. Returns whether that window is ours; the cursor
// position is written to point.
static bool cursor_surface(CGPoint* point, struct bar** bar_out,
                                           struct popup** popup_out) {
  *bar_out = NULL;
  *popup_out = NULL;

  CGEventRef event = CGEventCreate(NULL);
  if (event) {
    *point = CGEventGetLocation(event);
    CFRelease(event);
  }

  CGPoint window_point;
  uint32_t wid = 0;
  int wcid = 0;
  bool own_window;
  if (SLSFindWindowAndOwner(g_connection, 0, 1, 0, point,
                            &window_point, &wid, &wcid) != kCGErrorSuccess) {
    wid = 0;
    own_window = bar_manager_get_bar_by_point(&g_bar_manager, *point)
                 || bar_manager_get_popup_by_point(&g_bar_manager, *point);
  } else {
    own_window = wcid == g_connection;
  }
  if (!own_window) return false;

  if (wid) {
    *bar_out = bar_manager_get_bar_by_wid(&g_bar_manager, wid);
    if (!*bar_out)
      *popup_out = bar_manager_get_popup_by_wid(&g_bar_manager, wid);

    struct bar_item* bar_item = NULL;
    if (!*bar_out && !*popup_out)
      bar_item = bar_manager_get_item_by_wid(&g_bar_manager, wid, NULL);
    if (bar_item && bar_item->parent)
      *popup_out = &bar_item->parent->popup;
  }

  if (!*bar_out && !*popup_out) {
    *bar_out = bar_manager_get_bar_by_point(&g_bar_manager, *point);
    if (!*bar_out)
      *popup_out = bar_manager_get_popup_by_point(&g_bar_manager, *point);
  }
  return true;
}

static void set_mouse_over(struct bar* bar, struct popup* popup) {
  bar_manager_clear_mouse_over(&g_bar_manager);
  if (bar) bar->mouse_over = true;
  else if (popup) popup->mouse_over = true;
}

// The popup of a bar item usually sits a few points away from its host, and
// a window placed right below the bar can overlap its edge. On the way from
// the host into the popup the pointer then crosses a strip that is over no
// window of ours. Treat the column from the host to the popup, across the
// width of the popup, as part of the popup.
static struct popup* popup_bridge_at_point(CGPoint point) {
  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];
    if (!bar_item->drawing || !bar_item->popup.drawing) continue;

    struct popup* popup = &bar_item->popup;
    if (popup->adid < 1) continue;

    struct window* host = bar_item_get_window(bar_item, popup->adid);
    if (!host) continue;

    CGRect popup_frame = { popup->window.origin, popup->window.frame.size };
    CGRect host_frame = { host->origin, host->frame.size };
    CGFloat top = fmin(CGRectGetMinY(host_frame), CGRectGetMinY(popup_frame));
    CGFloat bottom = fmax(CGRectGetMaxY(host_frame),
                          CGRectGetMaxY(popup_frame));

    CGRect bridge = { { popup_frame.origin.x, top },
                      { popup_frame.size.width, bottom - top } };
    if (CGRectContainsPoint(bridge, point)) return popup;
  }
  return NULL;
}

// No window of ours reports the pointer leaving the bridge, so poll the
// cursor while it rests there.
static CFRunLoopTimerRef g_bridge_timer = NULL;

static void bridge_watch_stop(void) {
  if (!g_bridge_timer) return;
  CFRunLoopTimerInvalidate(g_bridge_timer);
  CFRelease(g_bridge_timer);
  g_bridge_timer = NULL;
}

static void bridge_watch_tick(CFRunLoopTimerRef timer, void* info) {
  CGPoint point = CGPointZero;
  struct bar* bar;
  struct popup* popup;
  bool own_window = cursor_surface(&point, &bar, &popup);

  if (!own_window && popup_bridge_at_point(point)) return;
  bridge_watch_stop();

  // The watch only runs while the pointer counts as inside a popup, so a
  // departure to a foreign window is a global exit, even if the popup or its
  // host was removed in the meantime.
  if (own_window) set_mouse_over(bar, popup);
  else bar_manager_handle_mouse_exited_global(&g_bar_manager);
  windows_unfreeze();
}

static void bridge_watch_start(void) {
  if (g_bridge_timer) return;
  g_bridge_timer = CFRunLoopTimerCreate(NULL,
                                        CFAbsoluteTimeGetCurrent() + 0.05,
                                        0.05,
                                        0,
                                        0,
                                        bridge_watch_tick,
                                        NULL                              );
  CFRunLoopAddTimer(CFRunLoopGetMain(), g_bridge_timer, kCFRunLoopCommonModes);
}

// The bar and popup windows sit behind the item and bracket windows that
// cover them, so the pointer often enters or leaves the bar through an item
// window alone. Resolve the global mouse state for those events as well,
// otherwise mouse.exited.global never fires after the pointer leaves
// through an item.
static void global_mouse_entered_from_item(struct bar* bar,
                                           struct popup* popup) {
  if (bar_manager_mouse_over_any_bar(&g_bar_manager)
      || bar_manager_mouse_over_any_popup(&g_bar_manager)) {
    return;
  }
  if (!bar && !popup) return;

  set_mouse_over(bar, popup);
  bar_manager_handle_mouse_entered_global(&g_bar_manager);
}

// Called for every exit from one of our windows. Moves the hover state to
// the bar or popup under the cursor, or sends mouse.exited.global and
// returns true when the cursor is over neither. The cursor position is
// written to point.
static bool global_mouse_exited(CGPoint* point) {
  struct bar* bar;
  struct popup* popup;
  bool own_window = cursor_surface(point, &bar, &popup);

  if (own_window) bridge_watch_stop();
  else {
    popup = popup_bridge_at_point(*point);
    if (popup) bridge_watch_start();
    else bridge_watch_stop();
  }

  if (!bar && !popup) {
    if (!bar_manager_mouse_over_any_bar(&g_bar_manager)
        && !bar_manager_mouse_over_any_popup(&g_bar_manager)) {
      return false;
    }
    bar_manager_handle_mouse_exited_global(&g_bar_manager);
    return true;
  }

  set_mouse_over(bar, popup);
  return false;
}

static void event_mouse_entered(void* context) {
  uint32_t wid = get_wid_from_cg_event(context);

  // An entry that is handled after the pointer already left again must not
  // mark the bar as hovered: no exit would follow to clear it.
  CGPoint point = CGEventGetLocation(context);
  struct bar* cursor_bar;
  struct popup* cursor_popup;
  bool own_window = cursor_surface(&point, &cursor_bar, &cursor_popup);

  struct bar* bar = bar_manager_get_bar_by_wid(&g_bar_manager, wid);
  if (bar) {
    // Handle global mouse entered event
    if (own_window
        && !bar->mouse_over
        && !bar_manager_mouse_over_any_popup(&g_bar_manager)) {
      bar->mouse_over = true;
      bar_manager_handle_mouse_entered_global(&g_bar_manager);
    }
    return;
  }

  struct popup* popup = bar_manager_get_popup_by_wid(&g_bar_manager, wid);
  if (popup) {
    // Handle global mouse entered event
    if (own_window
        && !popup->mouse_over
        && !bar_manager_mouse_over_any_bar(&g_bar_manager)) {
      popup->mouse_over = true;
      bar_manager_handle_mouse_entered_global(&g_bar_manager);
    }
    return;
  }

  struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                          wid,
                                                          NULL          );

  if (bar_item && own_window)
    global_mouse_entered_from_item(cursor_bar, cursor_popup);
  bar_manager_handle_mouse_entered(&g_bar_manager, bar_item);
}

static void event_mouse_exited(void* context) {
  uint32_t wid = get_wid_from_cg_event(context);
  CGPoint point = CGEventGetLocation(context);

  struct popup* popup = NULL;
  if (bar_manager_get_bar_by_wid(&g_bar_manager, wid)
      || (popup = bar_manager_get_popup_by_wid(&g_bar_manager, wid))) {
    bool exited = global_mouse_exited(&point);

    // Leaving a popup for the bar exits its host, unless the pointer is
    // back on the host itself.
    if (!exited && popup
        && !bar_manager_get_popup_by_point(&g_bar_manager, point)
        && bar_manager_get_bar_by_point(&g_bar_manager, point)) {
      bool has_complex_mask
              = popup->host->update_mask & (UPDATE_MOUSE_EXITED
                                            | UPDATE_EXITED_GLOBAL);
      if (has_complex_mask
          && bar_manager_get_item_by_point(&g_bar_manager, point, NULL)
              != popup->host) {
        bar_manager_handle_mouse_exited(&g_bar_manager, popup->host);
      }
    }
    return;
  }

  struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                          wid,
                                                          NULL          );
  if (!bar_item) return;

  // mouse.exited.global already sends mouse.exited to every item.
  bool exited = global_mouse_exited(&point);
  if (exited) return;

  // A queued exit can arrive after a global exit already notified every item.
  if (!bar_item->mouse_over) return;

  if (bar_item->update_mask & UPDATE_EXITED_GLOBAL
      && bar_manager_get_popup_by_point(&g_bar_manager, point)
         == &bar_item->popup) {
    return;
  }
  bar_manager_handle_mouse_exited(&g_bar_manager, bar_item);
}

#define SCROLL_TIMEOUT 150000000
struct {
  uint64_t timestamp;
  int delta_y;
} g_scroll_info;

static void event_mouse_scrolled(void* context) {
  CGPoint point = CGEventGetLocation(context);
  uint32_t wid = get_wid_from_cg_event(context);
  int scroll_delta
    = CGEventGetIntegerValueField(context,
        kCGScrollWheelEventDeltaAxis1);
  uint32_t modifier_keys = CGEventGetFlags(context);

  uint64_t event_time = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW_APPROX);
  if (g_scroll_info.timestamp + SCROLL_TIMEOUT > event_time) {
    g_scroll_info.delta_y += scroll_delta;
    return;
  } else {
    if (g_scroll_info.timestamp + 2*SCROLL_TIMEOUT < event_time)
      g_scroll_info.delta_y = 0;
    g_scroll_info.timestamp
      = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW_APPROX);
  }

  struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                          wid,
                                                          NULL           );

  if (!bar_item || bar_item->type == BAR_COMPONENT_GROUP) {
    bar_item = bar_manager_get_item_by_point(&g_bar_manager, point, NULL);
  }

  if (!bar_item) {
    struct bar* bar = bar_manager_get_bar_by_wid(&g_bar_manager, wid);
    if (bar) {
      // Handle global mouse scrolled event
      if (bar->mouse_over
          && !bar_manager_mouse_over_any_popup(&g_bar_manager)) {
        bar_manager_handle_mouse_scrolled_global(&g_bar_manager,
                                                 scroll_delta
                                                 + g_scroll_info.delta_y,
                                                 bar->adid,
                                                 modifier_keys           );
      }

      g_scroll_info.delta_y = 0;
      return;
    }

    struct popup* popup = bar_manager_get_popup_by_wid(&g_bar_manager, wid);
    if (popup) {
      // Handle global mouse scrolled event
      if (popup->mouse_over
          && !bar_manager_mouse_over_any_bar(&g_bar_manager)) {
        bar_manager_handle_mouse_scrolled_global(&g_bar_manager,
                                                 scroll_delta
                                                 + g_scroll_info.delta_y,
                                                 popup->adid,
                                                 modifier_keys           );
      }

      g_scroll_info.delta_y = 0;
      return;
    }
  }

  bar_item_on_scroll(bar_item,
                     scroll_delta + g_scroll_info.delta_y,
                     modifier_keys                        );

  if (bar_item && bar_item->needs_update)
    bar_manager_refresh(&g_bar_manager, false);

  g_scroll_info.delta_y = 0;
}


static void event_volume_changed(void* context) {
  bar_manager_handle_volume_change(&g_bar_manager, *(float*)context);
}

static void event_wifi_changed(void* context) {
  bar_manager_handle_wifi_change(&g_bar_manager, (char*)context);
}

static void event_brightness_changed(void* context) {
  bar_manager_handle_brightness_change(&g_bar_manager, *(float*)context);
}

static void event_power_source_changed(void* context) {
  bar_manager_handle_power_source_change(&g_bar_manager, (char*)context);
}

static void event_media_changed(void* context) {
  bar_manager_handle_media_change(&g_bar_manager, (char*)context);
}

static void event_cover_changed(void* context) {
  bar_manager_handle_media_cover_change(&g_bar_manager, (CGImageRef)context);
}

static void event_space_windows_changed(void* context) {
  bar_manager_handle_space_windows_change(&g_bar_manager, (char*)context);
}

static void event_hotload(void* context) {
  bar_manager_destroy(&g_bar_manager);
  bar_manager_init(&g_bar_manager);
  bar_manager_begin(&g_bar_manager);
  exec_config_file();
}

typedef void callback_type(void*);
static callback_type* event_handler[] = {
  [APPLICATION_FRONT_SWITCHED] = event_application_front_switched,
  [SPACE_CHANGED]              = event_space_changed,
  [DISPLAY_ADDED]              = event_display_added,
  [DISPLAY_REMOVED]            = event_display_removed,
  [DISPLAY_MOVED]              = event_display_moved,
  [DISPLAY_RESIZED]            = event_display_resized,
  [DISPLAY_CHANGED]            = event_display_changed,
  [MOUSE_UP]                   = event_mouse_up,
  [MOUSE_DRAGGED]              = event_mouse_dragged,
  [MOUSE_ENTERED]              = event_mouse_entered,
  [MOUSE_EXITED]               = event_mouse_exited,
  [MOUSE_SCROLLED]             = event_mouse_scrolled,
  [VOLUME_CHANGED]             = event_volume_changed,
  [WIFI_CHANGED]               = event_wifi_changed,
  [BRIGHTNESS_CHANGED]         = event_brightness_changed,
  [POWER_SOURCE_CHANGED]       = event_power_source_changed,
  [MEDIA_CHANGED]              = event_media_changed,
  [COVER_CHANGED]              = event_cover_changed,
  [DISTRIBUTED_NOTIFICATION]   = event_distributed_notification,
  [MENU_BAR_HIDDEN_CHANGED]    = event_menu_bar_hidden_changed,
  [SYSTEM_WOKE]                = event_system_woke,
  [SYSTEM_WILL_SLEEP]          = event_system_will_sleep,
  [SHELL_REFRESH]              = event_shell_refresh,
  [ANIMATOR_REFRESH]           = event_animator_refresh,
  [MACH_MESSAGE]               = event_mach_message,
  [HOTLOAD]                    = event_hotload,
  [SPACE_WINDOWS_CHANGED]      = event_space_windows_changed,
};


void event_execute(struct event* event) {
  if (g_space_management_mode != 1) {
    bar_manager_poll_active_display(&g_bar_manager);
  }
  event_handler[event->type](event->context);
  windows_unfreeze();
}

void event_post(struct event* event) {
  if (event->type == EVENT_TYPE_UNKNOWN) return;

  if (pthread_main_np() == 0) {
    dispatch_sync(dispatch_get_main_queue(), ^{ event_execute(event); });
  } else event_execute(event); 
}
