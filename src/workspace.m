#include "workspace.h"
#include "misc/helpers.h"

extern struct event_loop g_event_loop;

void workspace_event_handler_init(void **context) {
    workspace_context *ws_context = [workspace_context alloc];
    *context = ws_context;
}

void workspace_event_handler_begin(void **context) {
    workspace_context *ws_context = *context;
    [ws_context init];
}

void workspace_event_handler_end(void *context) {
    workspace_context *ws_context = (workspace_context *) context;
    [ws_context dealloc];
}

void workspace_create_custom_observer (void **context, char* notification) {
    workspace_context *ws_context = *context;
    [ws_context addCustomObserver:@(notification)];
}

@implementation workspace_context
- (id)init {
    if ((self = [super init])) {
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(activeDisplayDidChange:)
                name:@"NSWorkspaceActiveDisplayDidChangeNotification"
                object:nil];
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(activeSpaceDidChange:)
                name:NSWorkspaceActiveSpaceDidChangeNotification
                object:nil];
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(appSwitched:)
                name:NSWorkspaceDidActivateApplicationNotification
                object:nil];
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                selector:@selector(didWake:)
                name:NSWorkspaceDidWakeNotification
                object:nil];
        [[NSDistributedNotificationCenter defaultCenter] addObserver:self
                selector:@selector(didChangeMenuBarHiding:)
                name:@"AppleInterfaceMenuBarHidingChangedNotification"
                object:nil];
    }

    return self;
}

- (void)addCustomObserver:(NSString *)name {
  [[NSDistributedNotificationCenter defaultCenter] addObserver:self
                                                  selector:@selector(allDistributedNotifications:)
                                                  name:name
                                                  object:nil];
}

- (void)dealloc {
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void) allDistributedNotifications:(NSNotification *)note {
    char* name = (char*)[[note name] UTF8String];
    struct event *event = event_create(&g_event_loop, DISTRIBUTED_NOTIFICATION, string_copy(name));
    event_loop_post(&g_event_loop, event);
}

- (void)didWake:(NSNotification *)notification {
    struct event *event = event_create(&g_event_loop, SYSTEM_WOKE, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)appSwitched:(NSNotification *)notification {
    struct event *event = event_create(&g_event_loop, APPLICATION_FRONT_SWITCHED, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)didChangeMenuBarHiding:(NSNotification *)notification {
    struct event *event = event_create(&g_event_loop, MENU_BAR_HIDDEN_CHANGED, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)activeDisplayDidChange:(NSNotification *)notification {
    struct event *event = event_create(&g_event_loop, DISPLAY_CHANGED, NULL);
    event_loop_post(&g_event_loop, event);
}

- (void)activeSpaceDidChange:(NSNotification *)notification {
    struct event *event = event_create(&g_event_loop, SPACE_CHANGED, NULL);
    event_loop_post(&g_event_loop, event);
}

@end
