#include "display.h"

extern int workspace_display_notch_height(uint32_t did);
extern int g_connection;
extern bool g_brightness_events;


float g_last_brightness = -1.f;
static void brightness_handler(void* notification_center, uint32_t did, void* name, const void* sender, CFDictionaryRef info) {
  float b = 0;
  float* brightness = &b;
  DisplayServicesGetBrightness(did, brightness);
  if (g_last_brightness < *brightness - 1e-2
     || g_last_brightness > *brightness + 1e-2) {
    g_last_brightness = *brightness;
    struct event event = { (void*) brightness, BRIGHTNESS_CHANGED };
    event_post(&event);
  }
}

static DISPLAY_EVENT_HANDLER(display_handler) {
    if (flags & kCGDisplayAddFlag) {
        struct event event = { (void *)(intptr_t) did, DISPLAY_ADDED };
        event_post(&event);

        if (g_brightness_events && DisplayServicesCanChangeBrightness(did))
          DisplayServicesRegisterForBrightnessChangeNotifications(did, did, (void*)brightness_handler);
    } else if (flags & kCGDisplayRemoveFlag) {
        struct event event = { (void *)(intptr_t) did, DISPLAY_REMOVED };
        event_post(&event);

        if (g_brightness_events && DisplayServicesCanChangeBrightness(did))
          DisplayServicesUnregisterForBrightnessChangeNotifications(did, did);
    } else if (flags & kCGDisplayMovedFlag) {
        struct event event = { (void *)(intptr_t) did, DISPLAY_MOVED };
        event_post(&event);
    } else if (flags & kCGDisplayDesktopShapeChangedFlag) {
        struct event event = { (void *)(intptr_t) did, DISPLAY_RESIZED };
        event_post(&event);
    }
}

CFStringRef display_uuid(uint32_t did) {
    CFUUIDRef uuid_ref = CGDisplayCreateUUIDFromDisplayID(did);
    if (!uuid_ref) return NULL;

    CFStringRef uuid_str = CFUUIDCreateString(NULL, uuid_ref);
    CFRelease(uuid_ref);

    return uuid_str;
}

CGRect display_bounds(uint32_t did) {
    return CGDisplayBounds(did);
}

uint64_t display_space_id(uint32_t did) {
    CFStringRef uuid = display_uuid(did);
    if (!uuid) return 0;

    uint64_t sid = SLSManagedDisplayGetCurrentSpace(g_connection, uuid);
    CFRelease(uuid);
    return sid;
}

uint64_t *display_space_list(uint32_t did, int *count) {
    CFStringRef uuid = display_uuid(did);
    if (!uuid) return NULL;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    if (!display_spaces_ref) return NULL;

    uint64_t *space_list = NULL;
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFStringRef identifier = CFDictionaryGetValue(display_ref,
                                                      CFSTR("Display Identifier"));

        if (!CFEqual(uuid, identifier)) continue;

        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        int spaces_count = CFArrayGetCount(spaces_ref);

        space_list = malloc(sizeof(uint64_t) * spaces_count);
        *count = spaces_count;

        for (int j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &space_list[j]);
        }
    }
    CFRelease(display_spaces_ref);
    CFRelease(uuid);

    return space_list;
}

int display_arrangement(uint32_t did) {
    if (display_active_display_count() == 1) {
      uint32_t result = 0;
      uint32_t count = 0;
      CGGetActiveDisplayList(1, &result, &count);
      if (did == result && count == 1) return 1;
      else return 0;
    }

    CFStringRef uuid = display_uuid(did);
    if (!uuid) return 0;

    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);
    if (!displays) {
      CFRelease(uuid);
      return 0;
    }

    int result = 0;
    int displays_count = CFArrayGetCount(displays);

    for (int i = 0; i < displays_count; ++i) {
        if (CFEqual(CFArrayGetValueAtIndex(displays, i), uuid)) {
            result = i + 1;
            break;
        }
    }
    CFRelease(displays);
    CFRelease(uuid);
    return result;
}

uint32_t display_main_display_id(void) {
  return CGMainDisplayID();
}

static CFStringRef display_active_display_uuid(void) {
  return SLSCopyActiveMenuBarDisplayIdentifier(g_connection);
}

uint32_t display_active_display_id(void) {
    if (display_active_display_count() == 1) {
      uint32_t did = 0;
      uint32_t count = 0;
      CGGetActiveDisplayList(1, &did, &count);
      if (count == 1) return did;
      else {
        printf("ERROR (id): No active display detected!\n");
        return 0;
      }
    }

    CFStringRef uuid = display_active_display_uuid();
    CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, uuid);
    uint32_t result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
    CFRelease(uuid_ref);
    CFRelease(uuid);
    return result;
}

CFStringRef display_arrangement_display_uuid(int arrangement) {
    CFStringRef result = NULL;
    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);

    int displays_count = CFArrayGetCount(displays);
    for (int i = 0; i < displays_count; ++i) {
        if ((i+1) != arrangement) continue;
        result = CFRetain(CFArrayGetValueAtIndex(displays, i));
        break;
    }

    CFRelease(displays);
    return result;
}

uint32_t display_arrangement_display_id(int arrangement) {
    uint32_t result = 0;
    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);

    int displays_count = CFArrayGetCount(displays);
    for (int i = 0; i < displays_count; ++i) {
        if ((i+1) != arrangement) continue;
        CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL,
                                                    CFArrayGetValueAtIndex(displays, i));
        result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
        CFRelease(uuid_ref);
        break;
    }

    CFRelease(displays);
    return result;
}

uint32_t display_active_display_adid(void) {
    if (display_active_display_count() == 1) return 1;

    CFStringRef uuid = display_active_display_uuid();
    CFArrayRef displays = SLSCopyManagedDisplays(g_connection);
    if (!displays) {
      CFRelease(uuid);
      return 0;
    }

    int result = 0;
    int displays_count = CFArrayGetCount(displays);

    for (int i = 0; i < displays_count; ++i) {
        if (CFEqual(CFArrayGetValueAtIndex(displays, i), uuid)) {
            result = i + 1;
            break;
        }
    }
    CFRelease(displays);
    CFRelease(uuid);
    return result;
}

bool display_menu_bar_visible(void) {
    int status = 0;
    SLSGetMenuBarAutohideEnabled(g_connection, &status);
    return !status;
}

CGRect display_menu_bar_rect(uint32_t did) {
  CGRect bounds = {};

  #ifdef __x86_64__
  SLSGetRevealedMenuBarBounds(&bounds, g_connection, display_space_id(did));
  #elif __arm64__
  int notch_height = workspace_display_notch_height(did);
  if (notch_height) {
      bounds.size.height = notch_height + 6;
  } else {
      bounds.size.height = 24;
  }

  bounds.size.width = CGDisplayPixelsWide(did);
  #endif

  return bounds;
}

uint32_t display_active_display_count(void) {
    uint32_t count;
    CGGetActiveDisplayList(0, NULL, &count);
    return count;
}

uint32_t *display_active_display_list(uint32_t *count) {
    int display_count = display_active_display_count();
    uint32_t *result = malloc(sizeof(uint32_t) * display_count);
    CGGetActiveDisplayList(display_count, result, count);
    return result;
}

bool display_begin() {
    return CGDisplayRegisterReconfigurationCallback(display_handler, NULL)
            == kCGErrorSuccess;
}

bool display_end() {
    return CGDisplayRemoveReconfigurationCallback(display_handler, NULL)
            == kCGErrorSuccess;
}

void forced_brightness_event() {
  g_last_brightness = -1.f;
  brightness_handler(NULL, display_active_display_id(), NULL, NULL, NULL);
}

void begin_receiving_brightness_events() {
  if (g_brightness_events) return;
  g_brightness_events = true;
  uint32_t count;
  uint32_t* result = display_active_display_list(&count);
  for (int i = 0; i < count; i++) {
    uint32_t did = *(result + i);
    if (DisplayServicesCanChangeBrightness(did)) {
      DisplayServicesRegisterForBrightnessChangeNotifications(did, did, (void*)brightness_handler);
    }
  }
}

