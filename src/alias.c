#include "alias.h"
#include "misc/helpers.h"
#include <string.h>

void alias_init(struct alias* alias, char* name) {
  alias->using_light_colors = true;
  alias->name = name;
  alias->wid = 0;
  alias->image_ref = NULL;
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
    
    CFNumberRef layer_ref = CFDictionaryGetValue(dictionary, kCGWindowLayer);
    if (!layer_ref) continue;

    uint64_t layer = 0;
    CFNumberGetValue(layer_ref, CFNumberGetType(layer_ref), &layer);
    if (layer != MENUBAR_LAYER) continue;

    CFNumberRef window_id_ref = CFDictionaryGetValue(dictionary, kCGWindowNumber);
    if (!window_id_ref) continue;
    CGWindowID window_id = 0;
    CFNumberGetValue(window_id_ref, CFNumberGetType(window_id_ref), &window_id);
    char* owner = cfstring_copy(owner_ref);
    char* name = cfstring_copy(name_ref);

    if (strcmp(alias->name, owner) != 0) { free(owner); free(name); continue; }
    alias->wid = window_id;

    free(owner);
    free(name);
    CFRelease(window_list);
    return;
  }
  alias->wid = 0;
  CFRelease(window_list);
}

bool alias_update_image(struct alias* alias) {
  if (alias->wid == 0) alias_find_window(alias); 
  if (alias->wid == 0) { alias->image_ref = NULL; return false; }
  if (alias->image_ref) CFRelease(alias->image_ref);
  alias->image_ref = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, 
                                             alias->wid, kCGWindowImageBestResolution);
  if (!alias->image_ref) return false;
  alias->size.x = CGImageGetWidth(alias->image_ref);
  alias->size.y = CGImageGetHeight(alias->image_ref);
  return true;
}
