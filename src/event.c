#include "event.h"

extern struct event_loop g_event_loop;
extern struct process_manager g_process_manager;
extern struct display_manager g_display_manager;
extern struct bar_manager g_bar_manager;
extern struct window_manager g_window_manager;
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
    event->param1 = 0;
    event->info = 0;
    return event;
}

struct event *event_create_p1(struct event_loop *event_loop, enum event_type type, void *context, int param1) {
    struct event *event = memory_pool_push(&event_loop->pool, struct event);
    event->type = type;
    event->context = context;
    event->param1 = param1;
    event->info = 0;
    return event;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISTRIBUTED_NOTIFICATION) {
    debug("%s\n", context);
    bar_manager_handle_notification(&g_bar_manager, context);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_FRONT_SWITCHED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_front_app_switch(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_SPACE_CHANGED) {
    debug("%s\n", __FUNCTION__);
    bar_manager_handle_space_change(&g_bar_manager);
    bar_manager_refresh(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_CHANGED) {
    g_display_manager.last_display_id = g_display_manager.current_display_id;
    g_display_manager.current_display_id = display_manager_active_display_id();
    debug("%s: %d\n", __FUNCTION__, g_display_manager.current_display_id);
    bar_manager_handle_display_change(&g_bar_manager);
    bar_manager_refresh(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_ADDED) {
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_REMOVED) {
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_MOVED) {
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_RESIZED) {
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED) {
    debug("%s:\n", __FUNCTION__);
    bar_manager_resize(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WOKE) {
    debug("%s:\n", __FUNCTION__);
    bar_manager_handle_system_woke(&g_bar_manager);
    bar_manager_refresh(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_SHELL_REFRESH) {
    bar_manager_script_update(&g_bar_manager, false);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DAEMON_MESSAGE) {
    FILE *rsp = fdopen(param1, "w");
    if (!rsp) goto out;

    if (g_verbose) {
        fprintf(stdout, "%s:", __FUNCTION__);
        for (char *message = context; *message;) {
            message += fprintf(stdout, " %s", message);
        }
        putc('\n', stdout);
        fflush(stdout);
    }

    handle_message(rsp, context);
    fflush(rsp);
    fclose(rsp);

out:
    socket_close(param1);
    free(context);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_UP) {
    CGPoint point = CGEventGetLocation(context);

    uint32_t sid = mission_control_index(display_space_id(display_manager_active_display_id()));
    debug("EVENT_HANDLER_MOUSE_UP: S#%d (x: %.0f, y: %.0f) -> ", sid, point.x, point.y);
    struct bar_item* bar_item = bar_manager_get_item_by_point(&g_bar_manager, point, sid);
    debug("item: %s\n", bar_item ? bar_item->name : "NULL");
    bar_item_on_click(bar_item);
    CFRelease(context);
    return EVENT_SUCCESS;
}
