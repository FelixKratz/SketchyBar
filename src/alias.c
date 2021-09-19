#include "alias.h"
#include "misc/helpers.h"
#include <string.h>

void alias_get_permission(struct alias* alias) {
  if (@available(macOS 10.15, *)) alias->permission = CGRequestScreenCaptureAccess();
}

void alias_init(struct alias* alias, char* name) {
  alias->using_light_colors = true;
  alias->name = name;
  alias->wid = 0;
  alias->image_ref = NULL;
  alias_get_permission(alias);
  alias_update_image(alias);
}

void alias_find_window(struct alias* alias) {
  CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID); 
  int window_count = CFArrayGetCount(window_list);

  for (int i = 0; i < window_count; ++i) {
    CFDictionaryRef dictionary = CFArrayGetValueAtIndex(window_list, i);
    if (!dictionary) continue;

    CFStringRef owner_ref = CFDictionaryGetValue(dictionary, kCGWindowOwnerName);
    CFStringRef name_ref = CFDictionaryGetValue(dictionary, kCGWindowName);
    if (!name_ref) continue;
    if (!owner_ref) continue;
    char* owner = cfstring_copy(owner_ref);
    if (!owner) continue;

    if (strcmp(alias->name, owner) != 0) { free(owner); continue; }
    free(owner);

    CFNumberRef layer_ref = CFDictionaryGetValue(dictionary, kCGWindowLayer);
    if (!layer_ref) continue;

    uint64_t layer = 0;
    CFNumberGetValue(layer_ref, CFNumberGetType(layer_ref), &layer);
    if (layer != MENUBAR_LAYER) continue;

    CFNumberRef window_id_ref = CFDictionaryGetValue(dictionary, kCGWindowNumber);
    if (!window_id_ref) continue;
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
  CGImageRelease(alias->image_ref);
  alias->image_ref = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, 
                                             alias->wid, kCGWindowImageBestResolution);
  if (!alias->image_ref) {
    alias->size.x = 0;
    alias->size.y = 0;
    return false;
  }

  alias->size.x = CGImageGetWidth(alias->image_ref);
  alias->size.y = CGImageGetHeight(alias->image_ref);
  return true;
}
