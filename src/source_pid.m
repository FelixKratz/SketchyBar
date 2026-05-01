#include "source_pid.h"
#include "misc/helpers.h"
#include <AppKit/AppKit.h>
#include <pthread.h>
#include <math.h>

// The Accessibility API attribute for the extras menu bar (status items area)
// This is documented in Apple's Accessibility Programming Guide
#define kAXExtrasMenuBarAttribute CFSTR("AXExtrasMenuBar")

// Cache for running applications and their extras menu bars
typedef struct {
    pid_t pid;
    char* name;
    AXUIElementRef app_element;
    AXUIElementRef extras_menu_bar;
    bool has_extras_menu_bar;
} cached_app_t;

static cached_app_t* g_cached_apps = NULL;
static int g_cached_apps_count = 0;
static pthread_mutex_t g_cache_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_initialized = false;
static uint64_t g_last_refresh_time = 0;
static const uint64_t CACHE_REFRESH_INTERVAL_NS = 5000000000ULL; // 5 seconds in nanoseconds

// Helper to calculate distance between two points
static double point_distance(CGPoint a, CGPoint b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

// Helper to get the center of a rect
static CGPoint rect_center(CGRect rect) {
    return CGPointMake(rect.origin.x + rect.size.width / 2.0,
                       rect.origin.y + rect.size.height / 2.0);
}

bool source_pid_needs_workaround(void) {
    // macOS Tahoe (26.x) changed menu bar item ownership to Control Center
    // Check at runtime using NSProcessInfo
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    // macOS Tahoe is version 26.x
    return version.majorVersion >= 26;
}

void source_pid_cache_init(void) {
    if (g_initialized) return;
    g_initialized = true;
    source_pid_cache_refresh();
}

static void free_cached_app(cached_app_t* app) {
    if (app->name) {
        free(app->name);
        app->name = NULL;
    }
    if (app->extras_menu_bar) {
        CFRelease(app->extras_menu_bar);
        app->extras_menu_bar = NULL;
    }
    if (app->app_element) {
        CFRelease(app->app_element);
        app->app_element = NULL;
    }
}

void source_pid_cache_refresh(void) {
    // Throttle refreshes to avoid performance issues
    uint64_t now = clock_gettime_nsec_np(CLOCK_MONOTONIC);

    pthread_mutex_lock(&g_cache_mutex);

    // Check throttle inside the lock to avoid race conditions
    if (g_last_refresh_time > 0 && (now - g_last_refresh_time) < CACHE_REFRESH_INTERVAL_NS) {
        pthread_mutex_unlock(&g_cache_mutex);
        return; // Skip refresh if we refreshed recently
    }

    g_last_refresh_time = now;

    // Free existing cache
    if (g_cached_apps) {
        for (int i = 0; i < g_cached_apps_count; i++) {
            free_cached_app(&g_cached_apps[i]);
        }
        free(g_cached_apps);
        g_cached_apps = NULL;
        g_cached_apps_count = 0;
    }

    // Get list of running applications
    NSArray* running_apps = [[NSWorkspace sharedWorkspace] runningApplications];
    g_cached_apps_count = (int)[running_apps count];
    g_cached_apps = calloc(g_cached_apps_count, sizeof(cached_app_t));

    int actual_count = 0;
    for (NSRunningApplication* app in running_apps) {
        // Skip apps that can't have menu bar items
        if (app.activationPolicy == NSApplicationActivationPolicyProhibited) {
            continue;
        }

        pid_t pid = app.processIdentifier;

        cached_app_t* cached = &g_cached_apps[actual_count];
        cached->pid = pid;
        cached->name = app.localizedName ? strdup([app.localizedName UTF8String]) : NULL;
        cached->app_element = AXUIElementCreateApplication(pid);
        cached->extras_menu_bar = NULL;
        cached->has_extras_menu_bar = false;

        actual_count++;
    }
    g_cached_apps_count = actual_count;

    pthread_mutex_unlock(&g_cache_mutex);
}

// Try to get the extras menu bar for an application (lazy initialization)
static AXUIElementRef get_extras_menu_bar(cached_app_t* app) {
    if (app->extras_menu_bar) {
        return app->extras_menu_bar;
    }

    if (app->has_extras_menu_bar == false && app->app_element == NULL) {
        return NULL;
    }

    // Check if accessibility is trusted
    if (!AXIsProcessTrusted()) {
        return NULL;
    }

    // Try to get the extras menu bar attribute
    AXUIElementRef extras_bar = NULL;
    AXError error = AXUIElementCopyAttributeValue(app->app_element,
                                                   kAXExtrasMenuBarAttribute,
                                                   (CFTypeRef*)&extras_bar);

    if (error == kAXErrorSuccess && extras_bar) {
        app->extras_menu_bar = extras_bar;
        app->has_extras_menu_bar = true;
        return extras_bar;
    }

    // Mark that we've tried and failed
    app->has_extras_menu_bar = false;
    return NULL;
}

// Get the frame of an accessibility element
static bool get_ax_frame(AXUIElementRef element, CGRect* out_frame) {
    AXValueRef position_value = NULL;
    AXValueRef size_value = NULL;
    CGPoint position;
    CGSize size;

    AXError err = AXUIElementCopyAttributeValue(element, kAXPositionAttribute,
                                                 (CFTypeRef*)&position_value);
    if (err != kAXErrorSuccess || !position_value) {
        return false;
    }

    err = AXUIElementCopyAttributeValue(element, kAXSizeAttribute,
                                         (CFTypeRef*)&size_value);
    if (err != kAXErrorSuccess || !size_value) {
        CFRelease(position_value);
        return false;
    }

    bool success = false;
    if (AXValueGetValue(position_value, kAXValueCGPointType, &position) &&
        AXValueGetValue(size_value, kAXValueCGSizeType, &size)) {
        out_frame->origin = position;
        out_frame->size = size;
        success = true;
    }

    CFRelease(position_value);
    CFRelease(size_value);
    return success;
}

// Check if an accessibility element is enabled
static bool is_ax_element_enabled(AXUIElementRef element) {
    CFBooleanRef enabled_ref = NULL;
    AXError err = AXUIElementCopyAttributeValue(element, kAXEnabledAttribute,
                                                 (CFTypeRef*)&enabled_ref);
    if (err != kAXErrorSuccess || !enabled_ref) {
        return false;
    }

    bool enabled = CFBooleanGetValue(enabled_ref);
    CFRelease(enabled_ref);
    return enabled;
}

// Get children of an accessibility element
static CFArrayRef get_ax_children(AXUIElementRef element) {
    CFArrayRef children = NULL;
    AXUIElementCopyAttributeValue(element, kAXChildrenAttribute,
                                   (CFTypeRef*)&children);
    return children;
}

pid_t source_pid_for_window(CGRect window_bounds) {
    if (!source_pid_needs_workaround()) {
        return 0;
    }

    if (!AXIsProcessTrusted()) {
        return 0;
    }

    source_pid_cache_init();

    pthread_mutex_lock(&g_cache_mutex);

    CGPoint window_center = rect_center(window_bounds);
    pid_t result_pid = 0;

    // Prioritize apps that already have a cached extras menu bar
    // by iterating through them first
    for (int pass = 0; pass < 2 && result_pid == 0; pass++) {
        for (int i = 0; i < g_cached_apps_count && result_pid == 0; i++) {
            cached_app_t* app = &g_cached_apps[i];

            // First pass: only check apps with cached extras menu bars
            // Second pass: try to get extras menu bars for remaining apps
            if (pass == 0 && !app->has_extras_menu_bar) continue;
            if (pass == 1 && app->has_extras_menu_bar) continue;

            AXUIElementRef extras_bar = get_extras_menu_bar(app);
            if (!extras_bar) continue;

            CFArrayRef children = get_ax_children(extras_bar);
            if (!children) continue;

            CFIndex child_count = CFArrayGetCount(children);
            for (CFIndex j = 0; j < child_count; j++) {
                AXUIElementRef child = (AXUIElementRef)CFArrayGetValueAtIndex(children, j);

                if (!is_ax_element_enabled(child)) continue;

                CGRect child_frame;
                if (!get_ax_frame(child, &child_frame)) continue;

                // Check if the centers are close enough (within 1 point)
                CGPoint child_center = rect_center(child_frame);
                double distance = point_distance(window_center, child_center);

                if (distance <= 1.0) {
                    result_pid = app->pid;
                    break;
                }
            }

            CFRelease(children);
        }
    }

    pthread_mutex_unlock(&g_cache_mutex);
    return result_pid;
}

char* source_name_for_window(CGRect window_bounds) {
    if (!source_pid_needs_workaround()) {
        return NULL;
    }

    pid_t source_pid = source_pid_for_window(window_bounds);
    if (source_pid == 0) {
        return NULL;
    }

    pthread_mutex_lock(&g_cache_mutex);

    char* result = NULL;
    for (int i = 0; i < g_cached_apps_count; i++) {
        if (g_cached_apps[i].pid == source_pid && g_cached_apps[i].name) {
            result = strdup(g_cached_apps[i].name);
            break;
        }
    }

    pthread_mutex_unlock(&g_cache_mutex);
    return result;
}
