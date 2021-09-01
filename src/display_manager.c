#include "display_manager.h"

extern struct window_manager g_window_manager;
extern int g_connection;

uint32_t display_manager_main_display_id(void) {
    return CGMainDisplayID();
}

CFStringRef display_manager_active_display_uuid(void) {
    return SLSCopyActiveMenuBarDisplayIdentifier(g_connection);
}

uint32_t display_manager_active_display_id(void) {
    uint32_t result = 0;
    CFStringRef uuid = display_manager_active_display_uuid();
    CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, uuid);
    result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
    CFRelease(uuid_ref);
    CFRelease(uuid);
    return result;
}

CFStringRef display_manager_dock_display_uuid(void) {
    CGRect dock = display_manager_dock_rect();
    return SLSCopyBestManagedDisplayForRect(g_connection, dock);
}

uint32_t display_manager_dock_display_id(void) {
    CFStringRef uuid = display_manager_dock_display_uuid();
    if (!uuid) return 0;

    CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, uuid);
    uint32_t result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
    CFRelease(uuid_ref);
    CFRelease(uuid);
    return result;
}

CFStringRef display_manager_cursor_display_uuid(void) {
    CGPoint cursor;
    SLSGetCurrentCursorLocation(g_connection, &cursor);
    return SLSCopyBestManagedDisplayForPoint(g_connection, cursor);
}

uint32_t display_manager_cursor_display_id(void) {
    CFStringRef uuid = display_manager_cursor_display_uuid();
    if (!uuid) return 0;

    CFUUIDRef uuid_ref = CFUUIDCreateFromString(NULL, uuid);
    uint32_t result = CGDisplayGetDisplayIDFromUUID(uuid_ref);
    CFRelease(uuid_ref);
    CFRelease(uuid);
    return result;
}

CFStringRef display_manager_arrangement_display_uuid(int arrangement) {
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

uint32_t display_manager_arrangement_display_id(int arrangement) {
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

uint32_t display_manager_first_display_id(void) {
    return display_manager_arrangement_display_id(1);
}

uint32_t display_manager_last_display_id(void) {
    int arrangement = display_manager_active_display_count();
    return display_manager_arrangement_display_id(arrangement);
}

bool display_manager_menu_bar_visible(void) {
    int status = 0;
    SLSGetMenuBarAutohideEnabled(g_connection, &status);
    return !status;
}

CGRect display_manager_menu_bar_rect(uint32_t did) {
    CGRect bounds = {};
    SLSGetRevealedMenuBarBounds(&bounds, g_connection, display_space_id(did));
    return bounds;
}

bool display_manager_dock_hidden(void) {
    return CoreDockGetAutoHideEnabled();
}

int display_manager_dock_orientation(void) {
    int pinning = 0;
    int orientation = 0;
    CoreDockGetOrientationAndPinning(&orientation, &pinning);
    return orientation;
}

CGRect display_manager_dock_rect(void) {
    int reason = 0;
    CGRect bounds = {};
    SLSGetDockRectWithReason(g_connection, &bounds, &reason);
    return bounds;
}

bool display_manager_active_display_is_animating(void) {
    CFStringRef uuid = display_manager_active_display_uuid();
    bool result = SLSManagedDisplayIsAnimating(g_connection, uuid);
    CFRelease(uuid);
    return result;
}

bool display_manager_display_is_animating(uint32_t did) {
    CFStringRef uuid = display_uuid(did);
    if (!uuid) return false;

    bool result = SLSManagedDisplayIsAnimating(g_connection, uuid);
    CFRelease(uuid);
    return result;
}

uint32_t display_manager_active_display_count(void) {
    uint32_t count;
    CGGetActiveDisplayList(0, NULL, &count);
    return count;
}

uint32_t *display_manager_active_display_list(uint32_t *count) {
    int display_count = display_manager_active_display_count();
    uint32_t *result = malloc(sizeof(uint32_t) * display_count);
    CGGetActiveDisplayList(display_count, result, count);
    return result;
}

bool display_manager_begin(struct display_manager *dm) {
    dm->current_display_id = display_manager_active_display_id();
    dm->last_display_id = dm->current_display_id;
    return CGDisplayRegisterReconfigurationCallback(display_handler, NULL) == kCGErrorSuccess;
}

bool display_manager_end(void) {
    return CGDisplayRemoveReconfigurationCallback(display_handler, NULL) == kCGErrorSuccess;
}
