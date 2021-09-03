#include "display_manager.h"

extern int g_connection;

uint32_t display_main_display_id(void) {
    return CGMainDisplayID();
}

CFStringRef display_active_display_uuid(void) {
    return SLSCopyActiveMenuBarDisplayIdentifier(g_connection);
}

uint32_t display_active_display_id(void) {
    uint32_t result = 0;
    CFStringRef uuid = display_active_display_uuid();
    CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, uuid);
    result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
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
        CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, CFArrayGetValueAtIndex(displays, i));
        result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
        CFRelease(uuid_ref);
        break;
    }

    CFRelease(displays);
    return result;
}

bool display_menu_bar_visible(void) {
    int status = 0;
    SLSGetMenuBarAutohideEnabled(g_connection, &status);
    return !status;
}

CGRect display_menu_bar_rect(uint32_t did) {
    CGRect bounds = {};
    SLSGetRevealedMenuBarBounds(&bounds, g_connection, display_space_id(did));
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
    return CGDisplayRegisterReconfigurationCallback(display_handler, NULL) == kCGErrorSuccess;
}

bool display_end() {
    return CGDisplayRemoveReconfigurationCallback(display_handler, NULL) == kCGErrorSuccess;
}
