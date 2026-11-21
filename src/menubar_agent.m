#include "menubar_agent.h"
#include "misc/helpers.h"
#include "workspace.h"
#include <AppKit/AppKit.h>
#include <ApplicationServices/ApplicationServices.h>

#define MENUBAR_AGENT_BUNDLE_ID @"com.apple.MenuBarAgent"
#define MENUBAR_AGENT_MESSAGING_TIMEOUT 0.25f
#define MENUBAR_AGENT_MAX_DEPTH 8
// All aliases are updated in the same refresh cycle, so one read of the
// accessibility tree is shared between them for a short time.
#define MENUBAR_AGENT_CACHE_NS (250ULL * 1000ULL * 1000ULL)

static struct menubar_agent_item* g_cached_items = NULL;
static uint32_t g_cached_count = 0;
static uint64_t g_cached_time = 0;

static pid_t menubar_agent_pid(void) {
  @autoreleasepool {
    NSArray* apps = [NSRunningApplication
               runningApplicationsWithBundleIdentifier:MENUBAR_AGENT_BUNDLE_ID];
    if ([apps count] == 0) return 0;
    return [(NSRunningApplication*)apps[0] processIdentifier];
  }
}

bool menubar_agent_available(void) {
  return menubar_agent_pid() > 0;
}

bool menubar_agent_trusted(bool prompt) {
  if (!prompt) return AXIsProcessTrusted();
  const void* keys[] = { kAXTrustedCheckOptionPrompt };
  const void* values[] = { kCFBooleanTrue };
  CFDictionaryRef options = CFDictionaryCreate(NULL,
                                               keys,
                                               values,
                                               1,
                                               &kCFTypeDictionaryKeyCallBacks,
                                               &kCFTypeDictionaryValueCallBacks);
  bool trusted = AXIsProcessTrustedWithOptions(options);
  CFRelease(options);
  return trusted;
}

static CFTypeRef ax_copy_attribute(AXUIElementRef element, CFStringRef attribute) {
  CFTypeRef value = NULL;
  if (AXUIElementCopyAttributeValue(element, attribute, &value)
      != kAXErrorSuccess) {
    return NULL;
  }
  return value;
}

static bool ax_get_frame(AXUIElementRef element, CGRect* frame) {
  CFTypeRef position_ref = ax_copy_attribute(element, kAXPositionAttribute);
  CFTypeRef size_ref = ax_copy_attribute(element, kAXSizeAttribute);
  bool success = false;
  CGPoint position;
  CGSize size;
  if (position_ref && size_ref
      && AXValueGetValue(position_ref, kAXValueCGPointType, &position)
      && AXValueGetValue(size_ref, kAXValueCGSizeType, &size)) {
    *frame = (CGRect){ position, size };
    success = true;
  }
  if (position_ref) CFRelease(position_ref);
  if (size_ref) CFRelease(size_ref);
  return success;
}

static CFArrayRef ax_copy_children(AXUIElementRef element) {
  CFTypeRef children = ax_copy_attribute(element, kAXChildrenAttribute);
  if (children && CFGetTypeID(children) != CFArrayGetTypeID()) {
    CFRelease(children);
    return NULL;
  }
  return children;
}

static char* ax_copy_identifier(AXUIElementRef element) {
  CFTypeRef identifier = ax_copy_attribute(element, kAXIdentifierAttribute);
  if (!identifier) return NULL;
  char* result = NULL;
  if (CFGetTypeID(identifier) == CFStringGetTypeID()) {
    result = cfstring_copy(identifier);
    // AppKit assigns generated identifiers that are not stable
    if (result && (result[0] == '\0' || strncmp(result, "_NS:", 4) == 0)) {
      free(result);
      result = NULL;
    }
  }
  CFRelease(identifier);
  return result;
}

// Walks down from one child of the MenuBarAgent window. The wrappers around
// an item belong to MenuBarAgent, the first element that does not belongs to
// the app owning the item. Items of MenuBarAgent itself (the system items)
// only consist of MenuBarAgent elements, the innermost of which carries an
// identifier like com.apple.menuextra.wifi.
static pid_t menubar_agent_resolve_owner(AXUIElementRef child,
                                         pid_t agent_pid,
                                         char** identifier,
                                         AXUIElementRef* item_element) {
  AXUIElementRef element = (AXUIElementRef)CFRetain(child);
  pid_t owner = agent_pid;
  if (identifier) *identifier = NULL;

  for (int depth = 0; depth < MENUBAR_AGENT_MAX_DEPTH; depth++) {
    if (identifier) {
      char* element_identifier = ax_copy_identifier(element);
      if (element_identifier) {
        if (*identifier) free(*identifier);
        *identifier = element_identifier;
      }
    }

    pid_t pid = 0;
    if (AXUIElementGetPid(element, &pid) != kAXErrorSuccess) break;
    if (pid != agent_pid) {
      owner = pid;
      break;
    }

    CFArrayRef children = ax_copy_children(element);
    if (!children || CFArrayGetCount(children) == 0) {
      if (children) CFRelease(children);
      break;
    }

    AXUIElementRef next = (AXUIElementRef)CFArrayGetValueAtIndex(children, 0);
    CFRetain(next);
    CFRelease(children);
    CFRelease(element);
    element = next;
  }

  if (item_element) *item_element = element;
  else CFRelease(element);
  return owner;
}

static uint32_t menubar_agent_window_id(pid_t agent_pid, CGRect frame) {
  CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionAll,
                                                      kCGNullWindowID        );
  if (!window_list) return 0;

  uint32_t wid = 0;
  int window_count = CFArrayGetCount(window_list);
  for (int i = 0; i < window_count; i++) {
    CFDictionaryRef dictionary = CFArrayGetValueAtIndex(window_list, i);
    CFNumberRef pid_ref = CFDictionaryGetValue(dictionary, kCGWindowOwnerPID);
    CFDictionaryRef bounds_ref = CFDictionaryGetValue(dictionary,
                                                      kCGWindowBounds);
    CFNumberRef wid_ref = CFDictionaryGetValue(dictionary, kCGWindowNumber);
    if (!pid_ref || !bounds_ref || !wid_ref) continue;

    pid_t pid = 0;
    CFNumberGetValue(pid_ref, kCFNumberIntType, &pid);
    if (pid != agent_pid) continue;

    CGRect bounds;
    if (!CGRectMakeWithDictionaryRepresentation(bounds_ref, &bounds)) continue;
    if (CGRectEqualToRect(CGRectIntegral(bounds), CGRectIntegral(frame))) {
      CFNumberGetValue(wid_ref, kCFNumberIntType, &wid);
      break;
    }
  }

  CFRelease(window_list);
  return wid;
}

// The agent has one window per display. Apple documents no order, so the
// window showing the bar of the main display is picked by its position.
static AXUIElementRef menubar_agent_copy_main_window(AXUIElementRef agent,
                                                     CGRect* window_frame) {
  CFArrayRef windows = (CFArrayRef)ax_copy_attribute(agent,
                                                     kAXWindowsAttribute);
  if (!windows) return NULL;
  if (CFGetTypeID(windows) != CFArrayGetTypeID()) {
    CFRelease(windows);
    return NULL;
  }

  CGRect display = CGDisplayBounds(CGMainDisplayID());
  AXUIElementRef result = NULL;
  int window_count = CFArrayGetCount(windows);
  for (int i = 0; i < window_count; i++) {
    AXUIElementRef window = (AXUIElementRef)CFArrayGetValueAtIndex(windows, i);
    CGRect frame;
    if (!ax_get_frame(window, &frame)) continue;
    if (!result || CGRectContainsPoint(display, CGPointMake(CGRectGetMidX(frame),
                                                            CGRectGetMidY(frame)))) {
      if (result) CFRelease(result);
      result = (AXUIElementRef)CFRetain(window);
      *window_frame = frame;
      if (CGRectContainsPoint(display, CGPointMake(CGRectGetMidX(frame),
                                                   CGRectGetMidY(frame)))) {
        break;
      }
    }
  }

  CFRelease(windows);
  return result;
}

static void menubar_agent_items_sort(struct menubar_agent_item* items,
                                     uint32_t count) {
  // Highest x position first, as for the window based item list
  for (uint32_t i = 1; i < count; i++) {
    struct menubar_agent_item item = items[i];
    int32_t j = i - 1;
    while (j >= 0 && items[j].frame.origin.x < item.frame.origin.x) {
      items[j + 1] = items[j];
      j--;
    }
    items[j + 1] = item;
  }
}

static uint32_t menubar_agent_read_items(struct menubar_agent_item** out) {
  *out = NULL;
  pid_t agent_pid = menubar_agent_pid();
  if (!agent_pid || !AXIsProcessTrusted()) return 0;

  AXUIElementRef agent = AXUIElementCreateApplication(agent_pid);
  if (!agent) return 0;
  AXUIElementSetMessagingTimeout(agent, MENUBAR_AGENT_MESSAGING_TIMEOUT);

  CGRect window_frame = CGRectNull;
  AXUIElementRef window = menubar_agent_copy_main_window(agent, &window_frame);
  CFRelease(agent);
  if (!window) return 0;

  uint32_t wid = menubar_agent_window_id(agent_pid, window_frame);
  CFArrayRef children = ax_copy_children(window);
  CFRelease(window);
  if (!wid || !children) {
    if (children) CFRelease(children);
    return 0;
  }

  int child_count = CFArrayGetCount(children);
  struct menubar_agent_item* items = calloc(child_count > 0 ? child_count : 1,
                                            sizeof(struct menubar_agent_item));
  uint32_t count = 0;
  for (int i = 0; i < child_count; i++) {
    AXUIElementRef child = (AXUIElementRef)CFArrayGetValueAtIndex(children, i);
    CGRect frame;
    // Items that are about to leave the bar are kept without a size for a
    // moment, they can not be captured.
    if (!ax_get_frame(child, &frame)
        || frame.size.width <= 0
        || frame.size.height <= 0) {
      continue;
    }

    char* identifier = NULL;
    pid_t owner = menubar_agent_resolve_owner(child,
                                              agent_pid,
                                              &identifier,
                                              NULL        );
    char* owner_name = workspace_copy_app_name_for_pid(owner);
    if (!owner_name) owner_name = string_copy("");

    struct menubar_agent_item* item = items + count++;
    item->owner = owner_name;
    item->name = identifier;
    item->pid = owner;
    item->wid = wid;
    item->frame = frame;
    item->crop = (CGRect){{ frame.origin.x - window_frame.origin.x,
                            frame.origin.y - window_frame.origin.y },
                          frame.size                                  };
  }
  CFRelease(children);

  menubar_agent_items_sort(items, count);

  // Items without an identifier are named Item-0, Item-1, ... per owner,
  // counted from the right edge of the bar.
  for (uint32_t i = 0; i < count; i++) {
    if (items[i].name) continue;
    uint32_t index = 0;
    for (uint32_t j = 0; j < i; j++) {
      if (!items[j].name || strncmp(items[j].name, "Item-", 5) == 0) {
        if (strcmp(items[i].owner, items[j].owner) == 0) index++;
      }
    }
    char name[32];
    snprintf(name, sizeof(name), "Item-%u", index);
    items[i].name = string_copy(name);
  }

  *out = items;
  return count;
}

void menubar_agent_items_free(struct menubar_agent_item* items, uint32_t count) {
  if (!items) return;
  for (uint32_t i = 0; i < count; i++) {
    if (items[i].owner) free(items[i].owner);
    if (items[i].name) free(items[i].name);
  }
  free(items);
}

uint32_t menubar_agent_copy_items(struct menubar_agent_item** items) {
  @autoreleasepool {
    uint64_t now = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW_APPROX);
    if (!g_cached_items || now - g_cached_time > MENUBAR_AGENT_CACHE_NS) {
      menubar_agent_items_free(g_cached_items, g_cached_count);
      g_cached_count = menubar_agent_read_items(&g_cached_items);
      g_cached_time = now;
    }

    *items = NULL;
    if (!g_cached_items || g_cached_count == 0) return 0;

    struct menubar_agent_item* copy = calloc(g_cached_count,
                                             sizeof(struct menubar_agent_item));
    for (uint32_t i = 0; i < g_cached_count; i++) {
      copy[i] = g_cached_items[i];
      copy[i].owner = string_copy(g_cached_items[i].owner);
      copy[i].name = string_copy(g_cached_items[i].name);
    }
    *items = copy;
    return g_cached_count;
  }
}

bool menubar_agent_press(CGRect frame) {
  @autoreleasepool {
    pid_t agent_pid = menubar_agent_pid();
    if (!agent_pid || !AXIsProcessTrusted()) return false;

    AXUIElementRef agent = AXUIElementCreateApplication(agent_pid);
    if (!agent) return false;
    AXUIElementSetMessagingTimeout(agent, MENUBAR_AGENT_MESSAGING_TIMEOUT);

    CGRect window_frame = CGRectNull;
    AXUIElementRef window = menubar_agent_copy_main_window(agent,
                                                           &window_frame);
    CFRelease(agent);
    if (!window) return false;

    CFArrayRef children = ax_copy_children(window);
    CFRelease(window);
    if (!children) return false;

    CGPoint center = CGPointMake(CGRectGetMidX(frame), CGRectGetMidY(frame));
    bool pressed = false;
    int child_count = CFArrayGetCount(children);
    for (int i = 0; i < child_count; i++) {
      AXUIElementRef child = (AXUIElementRef)CFArrayGetValueAtIndex(children,
                                                                    i       );
      CGRect child_frame;
      if (!ax_get_frame(child, &child_frame)
          || !CGRectContainsPoint(child_frame, center)) {
        continue;
      }

      AXUIElementRef element = NULL;
      menubar_agent_resolve_owner(child, agent_pid, NULL, &element);
      if (!element) break;

      pressed = AXUIElementPerformAction(element, kAXPressAction)
                == kAXErrorSuccess;
      CFRelease(element);
      break;
    }

    CFRelease(children);
    return pressed;
  }
}
