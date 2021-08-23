#ifndef EVENT_LOOP_EVENT_H
#define EVENT_LOOP_EVENT_H

extern OSStatus SLSFindWindowByGeometry(int cid, int zero, int one, int zero_again, CGPoint *screen_point, CGPoint *window_point, uint32_t *wid, int *wcid);

#define EVENT_CALLBACK(name) uint32_t name(void *context, int param1)
typedef EVENT_CALLBACK(event_callback);

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_FRONT_SWITCHED);
static EVENT_CALLBACK(EVENT_HANDLER_SPACE_CHANGED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_ADDED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_REMOVED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_MOVED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_RESIZED);
static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_CHANGED);
static EVENT_CALLBACK(EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED);
static EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WOKE);
static EVENT_CALLBACK(EVENT_HANDLER_SHELL_REFRESH);
static EVENT_CALLBACK(EVENT_HANDLER_DAEMON_MESSAGE);
static EVENT_CALLBACK(EVENT_HANDLER_MOUSE_UP);

#define EVENT_QUEUED     0x0
#define EVENT_PROCESSED  0x1

#define EVENT_SUCCESS      0x0
#define EVENT_FAILURE      0x1
#define EVENT_MOUSE_IGNORE 0x2

#define event_status(e) ((e)  & 0x1)
#define event_result(e) ((e) >> 0x1)

enum event_type
{
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
    SHELL_REFRESH,
    DAEMON_MESSAGE,
    MOUSE_UP,

    EVENT_TYPE_COUNT
};

static const char *event_type_str[] =
{
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
    [SHELL_REFRESH]                  = "shell_refresh",
    [DAEMON_MESSAGE]                 = "daemon_message",
    [MOUSE_UP]                       = "mouse_up",

    [EVENT_TYPE_COUNT]               = "event_type_count"
};

static event_callback *event_handler[] =
{
    [APPLICATION_FRONT_SWITCHED]     = EVENT_HANDLER_APPLICATION_FRONT_SWITCHED,
    [SPACE_CHANGED]                  = EVENT_HANDLER_SPACE_CHANGED,
    [DISPLAY_ADDED]                  = EVENT_HANDLER_DISPLAY_ADDED,
    [DISPLAY_REMOVED]                = EVENT_HANDLER_DISPLAY_REMOVED,
    [DISPLAY_MOVED]                  = EVENT_HANDLER_DISPLAY_MOVED,
    [DISPLAY_RESIZED]                = EVENT_HANDLER_DISPLAY_RESIZED,
    [DISPLAY_CHANGED]                = EVENT_HANDLER_DISPLAY_CHANGED,
    [MOUSE_UP]                       = EVENT_HANDLER_MOUSE_UP,

    [MENU_BAR_HIDDEN_CHANGED]        = EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED,
    [SYSTEM_WOKE]                    = EVENT_HANDLER_SYSTEM_WOKE,
    [SHELL_REFRESH]                  = EVENT_HANDLER_SHELL_REFRESH,
    [DAEMON_MESSAGE]                 = EVENT_HANDLER_DAEMON_MESSAGE,
};

struct event
{
    void *context;
    volatile uint32_t *info;
    enum event_type type;
    int param1;
};

struct event *event_create(struct event_loop *event_loop, enum event_type type, void *context);
struct event *event_create_p1(struct event_loop *event_loop, enum event_type type, void *context, int param1);
void event_destroy(struct event_loop *event_loop, struct event *event);
enum event_type event_type_from_string(const char *str);

#endif
