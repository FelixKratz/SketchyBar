#pragma once
#include <mach/mach.h>
#include "misc/memory_pool.h"
#include "bar_manager.h"
#include "message.h"

struct event_loop;

extern uint32_t get_window_id_from_cg_event(CGEventRef cgevent);

#define EVENT_CALLBACK(name) uint32_t name(void *context)
typedef EVENT_CALLBACK(event_callback);

EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_FRONT_SWITCHED);
EVENT_CALLBACK(EVENT_HANDLER_SPACE_CHANGED);
EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_ADDED);
EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_REMOVED);
EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_MOVED);
EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_RESIZED);
EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_CHANGED);
EVENT_CALLBACK(EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED);
EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WOKE);
EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WILL_SLEEP);
EVENT_CALLBACK(EVENT_HANDLER_SHELL_REFRESH);
EVENT_CALLBACK(EVENT_HANDLER_ANIMATOR_REFRESH);
EVENT_CALLBACK(EVENT_HANDLER_MACH_MESSAGE);
EVENT_CALLBACK(EVENT_HANDLER_MOUSE_UP);
EVENT_CALLBACK(EVENT_HANDLER_MOUSE_DRAGGED);
EVENT_CALLBACK(EVENT_HANDLER_MOUSE_ENTERED);
EVENT_CALLBACK(EVENT_HANDLER_MOUSE_EXITED);
EVENT_CALLBACK(EVENT_HANDLER_VOLUME_CHANGED);
EVENT_CALLBACK(EVENT_HANDLER_WIFI_CHANGED);
EVENT_CALLBACK(EVENT_HANDLER_BRIGHTNESS_CHANGED);
EVENT_CALLBACK(EVENT_HANDLER_POWER_SOURCE_CHANGED);
EVENT_CALLBACK(EVENT_HANDLER_MEDIA_CHANGED);
EVENT_CALLBACK(EVENT_HANDLER_DISTRIBUTED_NOTIFICATION);

#define EVENT_QUEUED     0x0
#define EVENT_PROCESSED  0x1

#define EVENT_SUCCESS      0x0
#define EVENT_FAILURE      0x1
#define EVENT_MOUSE_IGNORE 0x2

#define event_status(e) ((e)  & 0x1)
#define event_result(e) ((e) >> 0x1)

enum event_type {
    EVENT_TYPE_UNKNOWN,
    APPLICATION_FRONT_SWITCHED,
    SPACE_CHANGED,
    DISPLAY_ADDED,
    DISPLAY_REMOVED,
    DISPLAY_MOVED,
    DISPLAY_RESIZED,
    DISPLAY_CHANGED,
    MENU_BAR_HIDDEN_CHANGED,
    SYSTEM_WOKE,
    SYSTEM_WILL_SLEEP,
    SHELL_REFRESH,
    ANIMATOR_REFRESH,
    MACH_MESSAGE,
    MOUSE_UP,
    MOUSE_DRAGGED,
    MOUSE_ENTERED,
    MOUSE_EXITED,
    VOLUME_CHANGED,
    WIFI_CHANGED,
    BRIGHTNESS_CHANGED,
    POWER_SOURCE_CHANGED,
    MEDIA_CHANGED,
    DISTRIBUTED_NOTIFICATION,

    EVENT_TYPE_COUNT
};

static const char *event_type_str[] = {
    [EVENT_TYPE_UNKNOWN]             = "event_type_unknown",

    [APPLICATION_FRONT_SWITCHED]     = "application_front_switched",
    [SPACE_CHANGED]                  = "space_changed",
    [DISPLAY_ADDED]                  = "display_added",
    [DISPLAY_REMOVED]                = "display_removed",
    [DISPLAY_MOVED]                  = "display_moved",
    [DISPLAY_RESIZED]                = "display_resized",
    [DISPLAY_CHANGED]                = "display_changed",
    [MENU_BAR_HIDDEN_CHANGED]        = "menu_bar_hidden_changed",
    [SYSTEM_WOKE]                    = "system_woke",
    [SYSTEM_WILL_SLEEP]              = "system_will_sleep",
    [SHELL_REFRESH]                  = "shell_refresh",
    [ANIMATOR_REFRESH]               = "animator_refresh",
    [MACH_MESSAGE]                   = "mach_message",
    [MOUSE_UP]                       = "mouse_up",
    [MOUSE_DRAGGED]                  = "mouse_dragged",
    [MOUSE_ENTERED]                  = "mouse_entered",
    [MOUSE_EXITED]                   = "mouse_exited",
    [VOLUME_CHANGED]                 = "volume_changed",
    [WIFI_CHANGED]                   = "wifi_changed",
    [BRIGHTNESS_CHANGED]             = "brightness_changed",
    [POWER_SOURCE_CHANGED]           = "power_source_changed",
    [MEDIA_CHANGED]                  = "media_changed",
    [DISTRIBUTED_NOTIFICATION]       = "distributed_notification",

    [EVENT_TYPE_COUNT]               = "event_type_count"
};

static event_callback *event_handler[] = {
    [APPLICATION_FRONT_SWITCHED]     = EVENT_HANDLER_APPLICATION_FRONT_SWITCHED,
    [SPACE_CHANGED]                  = EVENT_HANDLER_SPACE_CHANGED,
    [DISPLAY_ADDED]                  = EVENT_HANDLER_DISPLAY_ADDED,
    [DISPLAY_REMOVED]                = EVENT_HANDLER_DISPLAY_REMOVED,
    [DISPLAY_MOVED]                  = EVENT_HANDLER_DISPLAY_MOVED,
    [DISPLAY_RESIZED]                = EVENT_HANDLER_DISPLAY_RESIZED,
    [DISPLAY_CHANGED]                = EVENT_HANDLER_DISPLAY_CHANGED,
    [MOUSE_UP]                       = EVENT_HANDLER_MOUSE_UP,
    [MOUSE_DRAGGED]                  = EVENT_HANDLER_MOUSE_DRAGGED,
    [MOUSE_ENTERED]                  = EVENT_HANDLER_MOUSE_ENTERED,
    [MOUSE_EXITED]                   = EVENT_HANDLER_MOUSE_EXITED,
    [VOLUME_CHANGED]                 = EVENT_HANDLER_VOLUME_CHANGED,
    [WIFI_CHANGED]                   = EVENT_HANDLER_WIFI_CHANGED,
    [BRIGHTNESS_CHANGED]             = EVENT_HANDLER_BRIGHTNESS_CHANGED,
    [POWER_SOURCE_CHANGED]           = EVENT_HANDLER_POWER_SOURCE_CHANGED,
    [MEDIA_CHANGED]                  = EVENT_HANDLER_MEDIA_CHANGED,
    [DISTRIBUTED_NOTIFICATION]       = EVENT_HANDLER_DISTRIBUTED_NOTIFICATION,

    [MENU_BAR_HIDDEN_CHANGED]        = EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED,
    [SYSTEM_WOKE]                    = EVENT_HANDLER_SYSTEM_WOKE,
    [SYSTEM_WILL_SLEEP]              = EVENT_HANDLER_SYSTEM_WILL_SLEEP,
    [SHELL_REFRESH]                  = EVENT_HANDLER_SHELL_REFRESH,
    [ANIMATOR_REFRESH]               = EVENT_HANDLER_ANIMATOR_REFRESH,
    [MACH_MESSAGE]                   = EVENT_HANDLER_MACH_MESSAGE,
};

struct event {
    void *context;
    volatile uint32_t *info;
    enum event_type type;
};

struct event *event_create(struct event_loop *event_loop, enum event_type type, void *context);
void event_destroy(struct event_loop *event_loop, struct event *event);
enum event_type event_type_from_string(const char *str);
