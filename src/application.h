#ifndef APPLICATION_H
#define APPLICATION_H

#define OBSERVER_CALLBACK(name) void name(AXObserverRef observer, AXUIElementRef element, CFStringRef notification, void *context)
typedef OBSERVER_CALLBACK(observer_callback);

#define AX_APPLICATION_WINDOW_FOCUSED_INDEX       0
#define AX_APPLICATION_WINDOW_TITLE_CHANGED_INDEX 1

#define AX_APPLICATION_WINDOW_FOCUSED       (1 << AX_APPLICATION_WINDOW_FOCUSED_INDEX)
#define AX_APPLICATION_WINDOW_TITLE_CHANGED (1 << AX_APPLICATION_WINDOW_TITLE_CHANGED_INDEX)
#define AX_APPLICATION_ALL                  (AX_APPLICATION_WINDOW_FOCUSED |\
                                             AX_APPLICATION_WINDOW_TITLE_CHANGED)
static CFStringRef ax_application_notification[] =
{
    [AX_APPLICATION_WINDOW_FOCUSED_INDEX]       = kAXFocusedWindowChangedNotification,
    [AX_APPLICATION_WINDOW_TITLE_CHANGED_INDEX] = kAXTitleChangedNotification,
};

struct application
{
    AXUIElementRef ref;
    ProcessSerialNumber psn;
    uint32_t pid;
    char *name;
    AXObserverRef observer_ref;
    uint8_t notification;
    bool is_observing;
    bool retry;
};

uint32_t application_focused_window(struct application *application);
bool application_observe(struct application *application);
void application_unobserve(struct application *application);
struct application *application_create(struct process *process);
void application_destroy(struct application *application);

#endif
