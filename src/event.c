#include "event.h"
#include "event_loop.h"

extern struct event_loop g_event_loop;
extern struct bar_manager g_bar_manager;
extern int g_connection;

enum event_type event_type_from_string(const char *str) {
    for (int i = EVENT_TYPE_UNKNOWN + 1; i < EVENT_TYPE_COUNT; ++i) {
        if (string_equals(str, event_type_str[i])) return i;
    }
    return EVENT_TYPE_UNKNOWN;
}

struct event *event_create(struct event_loop *event_loop, enum event_type type, void *context) {
    struct event *event = memory_pool_push(&event_loop->pool, struct event);
    event->type = type;
    event->context = context;
    event->info = 0;
    return event;
}

EVENT_CALLBACK(EVENT_HANDLER_DISTRIBUTED_NOTIFICATION) {
    debug("%s\n", context);
    bar_manager_handle_notification(&g_bar_manager, context);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_FRONT_SWITCHED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_front_app_switch(&g_bar_manager, context);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_SPACE_CHANGED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_space_change(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_CHANGED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_display_change(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_ADDED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_display_change(&g_bar_manager);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_REMOVED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_display_change(&g_bar_manager);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_MOVED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_display_change(&g_bar_manager);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_RESIZED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_display_change(&g_bar_manager);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED) {
    debug("%s:\n", __FUNCTION__);
    bar_manager_resize(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WOKE) {
    debug("%s:\n", __FUNCTION__);
    bar_manager_handle_system_woke(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WILL_SLEEP) {
    debug("%s:\n", __FUNCTION__);
    bar_manager_handle_system_will_sleep(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_SHELL_REFRESH) {
    debug("%s\n", __FUNCTION__);
    bar_manager_update(&g_bar_manager, false);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_ANIMATOR_REFRESH) {
    debug("%s\n", __FUNCTION__);
    bar_manager_animator_refresh(&g_bar_manager);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_MACH_MESSAGE) {
    debug("%s\n", __FUNCTION__);

    if (context) handle_message_mach(context);
    mach_msg_destroy(&((struct mach_buffer*) context)->message.header);
    free(context);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_MOUSE_UP) {
    debug("%s\n", __FUNCTION__);
    CGPoint point = CGEventGetLocation(context);
    CGEventType type = CGEventGetType(context);
    uint32_t modifier_keys = CGEventGetFlags(context);
    uint32_t adid = display_arrangement(display_active_display_id());

    printf("EVENT_HANDLER_MOUSE_UP: D#%d (x: %.0f, y: %.0f) -> ",
          adid,
          point.x,
          point.y                                               );
    struct bar_item* bar_item = bar_manager_get_item_by_point(&g_bar_manager,
                                                              point,
                                                              adid           );

    printf("item: %s\n", bar_item ? bar_item->name : "NULL");
    bar_item_on_click(bar_item, type, modifier_keys);
    CFRelease(context);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_MOUSE_ENTERED) {
    debug("%s\n", __FUNCTION__);
    CGPoint point = CGEventGetLocation(context);
    uint32_t adid = display_arrangement(display_active_display_id());

    printf("EVENT_HANDLER_MOUSE_ENTERED: D#%d (x: %.0f, y: %.0f) -> ",
          adid,
          point.x,
          point.y                                                    );
    struct bar_item* bar_item = bar_manager_get_item_by_point(&g_bar_manager,
                                                              point,
                                                              adid           );

    printf("item: %s\n", bar_item ? bar_item->name : "NULL");
    bar_manager_handle_mouse_entered(&g_bar_manager, bar_item);
    CFRelease(context);
    return EVENT_SUCCESS;
}

EVENT_CALLBACK(EVENT_HANDLER_MOUSE_EXITED) {
    debug("%s\n", __FUNCTION__);
    printf("EVENT_HANDLER_MOUSE_EXITED \n");
    bar_manager_handle_mouse_exited(&g_bar_manager);
    CFRelease(context);
    return EVENT_SUCCESS;
}
