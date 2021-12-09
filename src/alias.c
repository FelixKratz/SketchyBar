#include "alias.h"
#include "misc/helpers.h"
#include <stdint.h>
#include <string.h>

extern CFArrayRef SLSHWCaptureWindowList(uint32_t cid, uint32_t* wid, uint32_t count, uint32_t flags);
extern void SLSCaptureWindowsContentsToRectWithOptions(uint32_t cid, uint32_t* wid, bool meh, CGRect bounds, uint32_t flags, CGImageRef* image);
extern int SLSGetScreenRectForWindow(uint32_t cid, uint32_t wid, CGRect* out);

void print_all_menu_items(FILE* rsp) {
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
  if (__builtin_available(macOS 11.0, *)) CGRequestScreenCaptureAccess();
#endif
  CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
  int window_count = CFArrayGetCount(window_list);
  fprintf(rsp, "Total Windows: %d \n", window_count);
  printf("Total Windows: %d \n", window_count);

  for (int i = 0; i < window_count; ++i) {
    CFDictionaryRef dictionary = CFArrayGetValueAtIndex(window_list, i);
    if (!dictionary) continue;

    CFStringRef owner_ref = CFDictionaryGetValue(dictionary, kCGWindowOwnerName);
    CFNumberRef owner_pid_ref = CFDictionaryGetValue(dictionary, kCGWindowOwnerPID);
    CFStringRef name_ref = CFDictionaryGetValue(dictionary, kCGWindowName);
    if (!name_ref) continue;
    if (!owner_ref) continue;
    if (!owner_pid_ref) continue;

    CFNumberRef layer_ref = CFDictionaryGetValue(dictionary, kCGWindowLayer);
    if (!layer_ref) continue;

    long long int layer = 0;
    CFNumberGetValue(layer_ref, CFNumberGetType(layer_ref), &layer);
    uint64_t owner_pid = 0;
    CFNumberGetValue(owner_pid_ref, CFNumberGetType(owner_pid_ref), &owner_pid);
    if (layer != MENUBAR_LAYER ) continue;
    char* owner = cfstring_copy(owner_ref);
    char* name = cfstring_copy(name_ref);

    if (strcmp(name, "") == 0) continue;
    fprintf(rsp, "Menu Item -> Owner: %s; with PID:%llu, Name: %s \n", owner, owner_pid, name);
    printf("Menu Item -> Owner: %s; with PID:%llu, Name: %s \n", owner, owner_pid, name);

    free(owner);
    free(name);
  }
  CFRelease(window_list);
}

void alias_get_permission(struct alias* alias) { 
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000 
  if (__builtin_available(macOS 11.0, *)) alias->permission = CGRequestScreenCaptureAccess();
#endif
}

void alias_init(struct alias* alias, char* owner, char* name) {
  alias->using_light_colors = true;
  alias->name = name;
  alias->owner = owner;
  alias->wid = 0;
  alias->image_ref = NULL;
  alias_get_permission(alias);
  alias_update_image(alias);
}

uint32_t alias_get_length(struct alias* alias) {
  if (alias->image_ref) return alias->bounds.size.width;
  return 0;
}

uint32_t alias_get_height(struct alias* alias) {
  if (alias->image_ref) return alias->bounds.size.height;
  return 0;
}

void alias_find_window(struct alias* alias) {
  CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
  int window_count = CFArrayGetCount(window_list);

  for (int i = 0; i < window_count; ++i) {
    CFDictionaryRef dictionary = CFArrayGetValueAtIndex(window_list, i);
    if (!dictionary) continue;

    CFStringRef owner_ref = CFDictionaryGetValue(dictionary, kCGWindowOwnerName);
    CFNumberRef owner_pid_ref = CFDictionaryGetValue(dictionary, kCGWindowOwnerPID);
    CFStringRef name_ref = CFDictionaryGetValue(dictionary, kCGWindowName);
    if (!name_ref) continue;
    if (!owner_ref) continue;
    char* owner = cfstring_copy(owner_ref);
    char* name = cfstring_copy(name_ref);

    if (!(alias->owner && strcmp(alias->owner, owner) == 0 && ((alias->name && strcmp(alias->name, name) == 0) || (!alias->name && strcmp(name, "") != 0)))) { free(owner); free(name); continue; }
    free(owner);
    free(name);

    CFNumberRef layer_ref = CFDictionaryGetValue(dictionary, kCGWindowLayer);
    if (!layer_ref) continue;

    uint64_t layer = 0;
    CFNumberGetValue(layer_ref, CFNumberGetType(layer_ref), &layer);
    if (layer != MENUBAR_LAYER) continue;

    CFNumberGetValue(owner_pid_ref, CFNumberGetType(owner_pid_ref), &alias->pid);

    CFNumberRef window_id_ref = CFDictionaryGetValue(dictionary, kCGWindowNumber);
    if (!window_id_ref) continue;
    CFDictionaryRef bounds = CFDictionaryGetValue(dictionary, kCGWindowBounds);
    if (!bounds) continue;
    CGRectMakeWithDictionaryRepresentation(bounds, &alias->bounds);
    CFNumberGetValue(window_id_ref, CFNumberGetType(window_id_ref), &alias->wid);

    CFRelease(window_list);
    return;
  }
  alias->wid = 0;
  CFRelease(window_list);
}

bool alias_update_image(struct alias* alias) {
  if (alias->wid == 0) alias_find_window(alias);
  if (alias->wid == 0) {
    alias->image_ref = NULL;
    return false;
  }

  SLSGetScreenRectForWindow(g_connection, alias->wid, &alias->bounds);
  alias->bounds.size.width = (uint32_t) (alias->bounds.size.width + 0.5);

  CGImageRef tmp_ref = NULL;
  SLSCaptureWindowsContentsToRectWithOptions(g_connection, &alias->wid, true, CGRectNull, 1 << 8, &tmp_ref);

  if (!tmp_ref) { alias->wid = 0; return false; }

  CGImageRelease(alias->image_ref);
  alias->image_ref = tmp_ref;

  return true;
}

void alias_draw(struct alias* alias, CGContextRef context) {
  if (!alias->image_ref) return;
  CGContextDrawImage(context, alias->bounds, alias->image_ref);
}

void alias_calculate_bounds(struct alias* alias, uint32_t x, uint32_t y) {
  alias->bounds.origin.x = x;
  alias->bounds.origin.y = y - alias->bounds.size.height / 2;
}
