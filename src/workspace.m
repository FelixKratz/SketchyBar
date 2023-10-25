#include "workspace.h"
#include "misc/helpers.h"

#include <AppKit/AppKit.h>
@interface workspace_context : NSObject {
}
- (id)init;
- (void)addCustomObserver:(NSString *)name;
@end

float workspace_get_scale() {
  return [[NSScreen mainScreen] backingScaleFactor];
}

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

int workspace_display_notch_height(uint32_t did)
{
  if (!CGDisplayIsBuiltin(did)) return 0;

  int height = 0;
  #if __MAC_OS_X_VERSION_MAX_ALLOWED >= 120000
  if (__builtin_available(macos 12.0, *)) {
    @autoreleasepool {
      for (NSScreen *screen in [NSScreen screens]) {
        if ([[[screen deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue] == did) {
            height = screen.safeAreaInsets.top;
        }
      }
    }
  }
  #endif

  return height;
}

void forced_front_app_event() {
  @autoreleasepool {
    NSString* name = [[[NSWorkspace sharedWorkspace] frontmostApplication] localizedName];
    const char* front_app = [name cStringUsingEncoding:NSUTF8StringEncoding];

    struct event event = { string_copy((char*)front_app),
                           APPLICATION_FRONT_SWITCHED    };
    event_post(&event);
  }
}

CGImageRef workspace_icon_for_app(char* app) {
  NSURL* path = [[NSWorkspace sharedWorkspace] URLForApplicationWithBundleIdentifier:[NSString stringWithUTF8String:app]];
  if (!path) return NULL;
  NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile:path.path];
  if (!image) return NULL;

  float scale = workspace_get_scale();
  NSRect rect = NSMakeRect( 0, 0, 16 * scale, 16 * scale);
  return (CGImageRef)CFRetain([image CGImageForProposedRect: &rect
                                                    context: NULL
                                                      hints: NULL]);
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
                selector:@selector(willSleep:)
                name:NSWorkspaceWillSleepNotification
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
  @autoreleasepool {
    struct notification* notification = notification_create();
    notification->name = string_copy((char*)[[note name] UTF8String]);
    if (note.userInfo && [NSJSONSerialization isValidJSONObject:note.userInfo]) {
      NSData* data = [NSJSONSerialization dataWithJSONObject:note.userInfo options:NSJSONWritingPrettyPrinted error:NULL];
      if (data && [data length] > 0) {
        char* info = malloc([data length] + 1);
        memcpy(info, [data bytes], [data length]);
        info[[data length]] = '\0';
        notification->info = info;
      }
    }

    struct event event = { notification, DISTRIBUTED_NOTIFICATION };
    event_post(&event);
  }
}

- (void)willSleep:(NSNotification *)notification {
    struct event event = { NULL, SYSTEM_WILL_SLEEP };
    event_post(&event);
}

- (void)didWake:(NSNotification *)notification {
    struct event event = { NULL, SYSTEM_WOKE };
    event_post(&event);
}

- (void)appSwitched:(NSNotification *)notification {
    @autoreleasepool {
      char* name = NULL;
      if (notification && notification.userInfo) {
        NSRunningApplication* app = [notification.userInfo objectForKey:NSWorkspaceApplicationKey];
        if (app) name = string_copy((char*)[[app localizedName] UTF8String]);
      }
      struct event event = { name, APPLICATION_FRONT_SWITCHED };
      event_post(&event);
    }
}

- (void)didChangeMenuBarHiding:(NSNotification *)notification {
    struct event event = { NULL, MENU_BAR_HIDDEN_CHANGED };
    event_post(&event);
}

- (void)activeDisplayDidChange:(NSNotification *)notification {
    struct event event = { NULL, DISPLAY_CHANGED };
    event_post(&event);
}

- (void)activeSpaceDidChange:(NSNotification *)notification {
    struct event event = { NULL, SPACE_CHANGED };
    event_post(&event);
}

@end
