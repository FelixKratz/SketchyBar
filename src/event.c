#include "event.h"
#include "hotload.h"

extern struct bar_manager g_bar_manager;
extern int g_connection;

static uint32_t event_distributed_notification(void* context) {
    bar_manager_handle_notification(&g_bar_manager, context);
    return EVENT_SUCCESS;
}

static uint32_t event_application_front_switched(void* context) {
    bar_manager_handle_front_app_switch(&g_bar_manager, context);
    return EVENT_SUCCESS;
}

static uint32_t event_space_changed(void* context) {
    bar_manager_handle_space_change(&g_bar_manager, false);
    return EVENT_SUCCESS;
}

static uint32_t event_display_changed(void* context) {
    bar_manager_handle_display_change(&g_bar_manager);
    return EVENT_SUCCESS;
}

static uint32_t event_display_added(void* context) {
    uint32_t did = (uint32_t)(intptr_t)context;
    bar_manager_display_added(&g_bar_manager, did);
    return EVENT_SUCCESS;
}

static uint32_t event_display_removed(void* context) {
    uint32_t did = (uint32_t)(intptr_t)context;
    bar_manager_display_removed(&g_bar_manager, did);
    return EVENT_SUCCESS;
}

static uint32_t event_display_moved(void* context) {
    uint32_t did = (uint32_t)(intptr_t)context;
    bar_manager_display_moved(&g_bar_manager, did);
    return EVENT_SUCCESS;
}

static uint32_t event_display_resized(void* context) {
    uint32_t did = (uint32_t)(intptr_t)context;
    bar_manager_display_resized(&g_bar_manager, did);
    return EVENT_SUCCESS;
}

static uint32_t event_menu_bar_hidden_changed(void* context) {
    bar_manager_resize(&g_bar_manager);
    g_bar_manager.bar_needs_update = true;
    bar_manager_refresh(&g_bar_manager, false);
    return EVENT_SUCCESS;
}

static uint32_t event_system_woke(void* context) {
    bar_manager_handle_system_woke(&g_bar_manager);
    return EVENT_SUCCESS;
}

static uint32_t event_system_will_sleep(void* context) {
    bar_manager_handle_system_will_sleep(&g_bar_manager);
    return EVENT_SUCCESS;
}

static uint32_t event_shell_refresh(void* context) {
    bar_manager_update(&g_bar_manager, false);
    return EVENT_SUCCESS;
}

static uint32_t event_animator_refresh(void* context) {
    bar_manager_animator_refresh(&g_bar_manager);
    return EVENT_SUCCESS;
}

static uint32_t event_mach_message(void* context) {
    handle_message_mach(context);
    return EVENT_SUCCESS;
}

static uint32_t event_mouse_up(void* context) {
    CGPoint point = CGEventGetLocation(context);
    uint32_t wid = get_wid_from_cg_event(context);
    CGEventType type = CGEventGetType(context);
    uint32_t mouse_button_code = CGEventGetIntegerValueField(context, kCGMouseEventButtonNumber);
    uint32_t modifier_keys = CGEventGetFlags(context);
    uint32_t adid = display_arrangement(display_active_display_id());

    struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                            wid,
                                                            adid           );

    struct bar* bar = bar_manager_get_bar_by_wid(&g_bar_manager, wid);
    struct popup* popup = bar_manager_get_popup_by_wid(&g_bar_manager, wid);
    if (!bar_item && !popup && !bar) {
      return EVENT_SUCCESS;
    }

    if (!bar_item || bar_item->type == BAR_COMPONENT_GROUP) {
      bar_item = bar_manager_get_item_by_point(&g_bar_manager, point, adid);
    }

    struct window* window = NULL;
    CGPoint point_in_window_coords = CGPointZero;
    if (bar_item) {
      window = bar_item_get_window(bar_item, adid);
      if (window) {
        point_in_window_coords.x = point.x - window->origin.x;
        point_in_window_coords.y = point.y - window->origin.y;
      }
    }

    bar_item_on_click(bar_item, type, mouse_button_code, modifier_keys, point_in_window_coords);

    if (bar_item && bar_item->needs_update)
      bar_manager_refresh(&g_bar_manager, false);

    return EVENT_SUCCESS;
}

static uint32_t event_mouse_dragged(void* context) {
    CGPoint point = CGEventGetLocation(context);
    uint32_t wid = get_wid_from_cg_event(context);
    uint32_t adid = display_arrangement(display_active_display_id());

    struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                            wid,
                                                            adid           );

    if (!bar_item || !bar_item->has_slider) {
      return EVENT_SUCCESS;
    }

    struct window* window = NULL;
    CGPoint point_in_window_coords = CGPointZero;
    if (bar_item) {
      window = bar_item_get_window(bar_item, adid);
      if (window) {
        point_in_window_coords.x = point.x - window->origin.x;
        point_in_window_coords.y = point.y - window->origin.y;
      }
    }

    bar_item_on_drag(bar_item, point_in_window_coords);

    if (bar_item->needs_update)
      bar_manager_refresh(&g_bar_manager, false);

    return EVENT_SUCCESS;
}

static uint32_t event_mouse_entered(void* context) {
    uint32_t wid = get_wid_from_cg_event(context);
    
    uint32_t adid = display_arrangement(display_active_display_id());

    struct bar* bar = bar_manager_get_bar_by_wid(&g_bar_manager, wid);
    if (bar) {
      // Handle global mouse entered event
      if (!bar->mouse_over
          && !bar_manager_mouse_over_any_popup(&g_bar_manager)) {
        bar->mouse_over = true;
        bar_manager_handle_mouse_entered_global(&g_bar_manager);
      }
      
      return EVENT_SUCCESS;
    }

    struct popup* popup = bar_manager_get_popup_by_wid(&g_bar_manager, wid);
    if (popup) {
      // Handle global mouse entered event
      if (!popup->mouse_over
          && !bar_manager_mouse_over_any_bar(&g_bar_manager)) {
        popup->mouse_over = true;
        bar_manager_handle_mouse_entered_global(&g_bar_manager);
      }
      
      return EVENT_SUCCESS;
    }
    struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                            wid,
                                                            adid           );

    if (!bar_item) {
      CGPoint point = CGEventGetLocation(context);
      bar_item = bar_manager_get_item_by_point(&g_bar_manager, point, adid);
    }

    bar_manager_handle_mouse_entered(&g_bar_manager, bar_item);
    return EVENT_SUCCESS;
}

static uint32_t event_mouse_exited(void* context) {
    uint32_t adid = display_arrangement(display_active_display_id());
    uint32_t wid = get_wid_from_cg_event(context);

    struct bar* bar,* bar_target;
    struct popup* popup,* popup_target;
    struct window* origin_window;
    bool over_target = false;

    CGPoint point = CGEventGetLocation(context);
    if ((bar = bar_manager_get_bar_by_wid(&g_bar_manager, wid))) {
      origin_window = &bar->window;
      popup_target = bar_manager_get_popup_by_point(&g_bar_manager,
                                                    point          );
      over_target = (popup_target != NULL);
    }
    else if ((popup = bar_manager_get_popup_by_wid(&g_bar_manager, wid))) {
      origin_window = &popup->window;
      bar_target = bar_manager_get_bar_by_point(&g_bar_manager, point);
      over_target = (bar_target != NULL);
    }

    if (bar || popup) {
      // Handle global mouse exited event
      CGRect frame = origin_window->frame;
      frame.origin = origin_window->origin;

      bool over_origin = CGRectContainsPoint(frame, point);

      if (!over_origin && !over_target) {
        if (bar) bar->mouse_over = false;
        else popup->mouse_over = false;
        bar_manager_handle_mouse_exited_global(&g_bar_manager);
      } else if (!over_origin && over_target) {
        if (bar) {
          bar->mouse_over = false;
          popup_target->mouse_over = true;
        }
        else {
          popup->mouse_over = false;
          bar_target->mouse_over = true;
        }
      }
      
      return EVENT_SUCCESS;
    }

    struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                            wid,
                                                            adid           );

    if (!bar_item || !(bar_item->update_mask & UPDATE_EXITED_GLOBAL)
        || (bar_manager_get_bar_by_point(&g_bar_manager, point)
            && bar_manager_get_popup_by_point(&g_bar_manager,
                                              point) != &bar_item->popup)) {
      bar_manager_handle_mouse_exited(&g_bar_manager, bar_item);
    }

    return EVENT_SUCCESS;
}

#define SCROLL_TIMEOUT 150000000
struct {
  uint64_t timestamp;
  int delta_y;

} g_scroll_info;

static uint32_t event_mouse_scrolled(void* context) {
    CGPoint point = CGEventGetLocation(context);
    uint32_t wid = get_wid_from_cg_event(context);
    int scroll_delta
                  = CGEventGetIntegerValueField(context,
                                                kCGScrollWheelEventDeltaAxis1);
    uint32_t adid = display_arrangement(display_active_display_id());

    uint64_t event_time = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW_APPROX);
    if (g_scroll_info.timestamp + SCROLL_TIMEOUT > event_time) {
      g_scroll_info.delta_y += scroll_delta;
      return EVENT_SUCCESS;
    } else {
      if (g_scroll_info.timestamp + 2*SCROLL_TIMEOUT < event_time)
        g_scroll_info.delta_y = 0;
      g_scroll_info.timestamp
                           = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW_APPROX);
    }
    

    struct bar* bar = bar_manager_get_bar_by_wid(&g_bar_manager, wid);
    if (bar) {
      // Handle global mouse scrolled event
      if (bar->mouse_over
          && !bar_manager_mouse_over_any_popup(&g_bar_manager)) {
        bar_manager_handle_mouse_scrolled_global(&g_bar_manager,
                                                 scroll_delta
                                                 + g_scroll_info.delta_y,
                                                 bar->adid);
      }

      g_scroll_info.delta_y = 0;
      return EVENT_SUCCESS;
    }

    struct popup* popup = bar_manager_get_popup_by_wid(&g_bar_manager, wid);
    if (popup) {
      // Handle global mouse scrolled event
      if (popup->mouse_over
          && !bar_manager_mouse_over_any_bar(&g_bar_manager)) {
        bar_manager_handle_mouse_scrolled_global(&g_bar_manager,
                                                 scroll_delta
                                                 + g_scroll_info.delta_y,
                                                 bar->adid);
      }

      g_scroll_info.delta_y = 0;
      return EVENT_SUCCESS;
    }

    struct bar_item* bar_item = bar_manager_get_item_by_wid(&g_bar_manager,
                                                            wid,
                                                            adid           );

    if (!bar_item || bar_item->type == BAR_COMPONENT_GROUP) {
      bar_item = bar_manager_get_item_by_point(&g_bar_manager, point, adid);
    }

    bar_item_on_scroll(bar_item, scroll_delta + g_scroll_info.delta_y);

    if (bar_item && bar_item->needs_update)
      bar_manager_refresh(&g_bar_manager, false);

    g_scroll_info.delta_y = 0;
    return EVENT_SUCCESS;
}


static uint32_t event_volume_changed(void* context) {
  bar_manager_handle_volume_change(&g_bar_manager, *(float*)context);
  return EVENT_SUCCESS;
}

static uint32_t event_wifi_changed(void* context) {
  bar_manager_handle_wifi_change(&g_bar_manager, (char*)context);
  return EVENT_SUCCESS;
}

static uint32_t event_brightness_changed(void* context) {
  bar_manager_handle_brightness_change(&g_bar_manager, *(float*)context);
  return EVENT_SUCCESS;
}

static uint32_t event_power_source_changed(void* context) {
  bar_manager_handle_power_source_change(&g_bar_manager, (char*)context);
  return EVENT_SUCCESS;
}

static uint32_t event_media_changed(void* context) {
  bar_manager_handle_media_change(&g_bar_manager, (char*)context);
  return EVENT_SUCCESS;
}

static uint32_t event_hotload(void* context) {
  bar_manager_destroy(&g_bar_manager);
  bar_manager_init(&g_bar_manager);
  bar_manager_begin(&g_bar_manager);
  exec_config_file();
  return EVENT_SUCCESS;
}

typedef uint32_t callback_type(void*);
static callback_type* event_handler[] = {
    [APPLICATION_FRONT_SWITCHED]     = event_application_front_switched,
    [SPACE_CHANGED]                  = event_space_changed,
    [DISPLAY_ADDED]                  = event_display_added,
    [DISPLAY_REMOVED]                = event_display_removed,
    [DISPLAY_MOVED]                  = event_display_moved,
    [DISPLAY_RESIZED]                = event_display_resized,
    [DISPLAY_CHANGED]                = event_display_changed,
    [MOUSE_UP]                       = event_mouse_up,
    [MOUSE_DRAGGED]                  = event_mouse_dragged,
    [MOUSE_ENTERED]                  = event_mouse_entered,
    [MOUSE_EXITED]                   = event_mouse_exited,
    [MOUSE_SCROLLED]                 = event_mouse_scrolled,
    [VOLUME_CHANGED]                 = event_volume_changed,
    [WIFI_CHANGED]                   = event_wifi_changed,
    [BRIGHTNESS_CHANGED]             = event_brightness_changed,
    [POWER_SOURCE_CHANGED]           = event_power_source_changed,
    [MEDIA_CHANGED]                  = event_media_changed,
    [DISTRIBUTED_NOTIFICATION]       = event_distributed_notification,

    [MENU_BAR_HIDDEN_CHANGED]        = event_menu_bar_hidden_changed,
    [SYSTEM_WOKE]                    = event_system_woke,
    [SYSTEM_WILL_SLEEP]              = event_system_will_sleep,
    [SHELL_REFRESH]                  = event_shell_refresh,
    [ANIMATOR_REFRESH]               = event_animator_refresh,
    [MACH_MESSAGE]                   = event_mach_message,
    [HOTLOAD]                        = event_hotload,
};

void event_post(struct event *event) {
  event_handler[event->type](event->context);
  windows_unfreeze();
}

