#include "alias.h"

extern void SLSCaptureWindowsContentsToRectWithOptions(uint32_t cid, uint64_t* wid, bool meh, CGRect bounds, uint32_t flags, CGImageRef* image);
extern int SLSGetScreenRectForWindow(uint32_t cid, uint32_t wid, CGRect* out);

void print_all_menu_items(FILE* rsp) {
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
  if (__builtin_available(macOS 11.0, *)) CGRequestScreenCaptureAccess();
#endif
  CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
  int window_count = CFArrayGetCount(window_list);

  printf("[\n");
  fprintf(rsp, "[\n");
  int counter = 0;
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
    if (counter++ > 0) {
      fprintf(rsp, ", \n");
      printf(", \n");
    }
    fprintf(rsp, "\t\"%s,%s\"", owner, name);
    printf("\t\"%s,%s\"", owner, name);

    free(owner);
    free(name);
  }
  printf("\n]\n");
  fprintf(rsp, "\n]\n");
  CFRelease(window_list);
}

void alias_get_permission(struct alias* alias) { 
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000 
  if (__builtin_available(macOS 11.0, *)) alias->permission = CGRequestScreenCaptureAccess();
#endif
}

void alias_init(struct alias* alias) {
  alias->name = NULL;
  alias->owner = NULL;
  alias->wid = 0;
  alias->color_override = false;
  alias->color = rgba_color_from_hex(0xffff0000);

  image_init(&alias->image);
}

void alias_setup(struct alias* alias, char* owner, char* name) {
  alias->name = name;
  alias->owner = owner;
  alias_get_permission(alias);
  alias_update_image(alias);
}

uint32_t alias_get_length(struct alias* alias) {
  if (alias->image.image_ref) return alias->image.bounds.size.width;
  return 0;
}

uint32_t alias_get_height(struct alias* alias) {
  if (alias->image.image_ref) return alias->image.bounds.size.height;
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
    CGRectMakeWithDictionaryRepresentation(bounds, &alias->image.bounds);
    CFNumberGetValue(window_id_ref, CFNumberGetType(window_id_ref), &alias->wid);

    CFRelease(window_list);
    return;
  }
  alias->wid = 0;
  CFRelease(window_list);
}

bool alias_update_image(struct alias* alias) {
  if (alias->wid == 0) alias_find_window(alias);
  if (alias->wid == 0) return false;

  CGRect bounds = CGRectNull;
  SLSGetScreenRectForWindow(g_connection, alias->wid, &bounds);
  bounds.size.width = (uint32_t) (bounds.size.width + 0.5);

  CGImageRef tmp_ref = NULL;
  SLSCaptureWindowsContentsToRectWithOptions(g_connection, &alias->wid, true, CGRectNull, 1 << 8, &tmp_ref);

  if (!tmp_ref) { alias->wid = 0; return false; }

  return image_set_image(&alias->image, tmp_ref, bounds, false);
}

void alias_draw(struct alias* alias, CGContextRef context) {
  if (alias->color_override) {
    CGContextSaveGState(context);
    image_draw(&alias->image, context);
    CGContextClipToMask(context, alias->image.bounds, alias->image.image_ref);
    CGContextSetRGBFillColor(context, alias->color.r, alias->color.g, alias->color.b, alias->color.a);
    CGContextFillRect(context, alias->image.bounds);
    CGContextRestoreGState(context);
  }
  else {
    image_draw(&alias->image, context);
  }
}

void alias_destroy(struct alias* alias) {
  image_destroy(&alias->image);
  if (alias->name) free(alias->name);
  if (alias->owner) free(alias->owner);
  alias->name = NULL;
  alias->owner = NULL;
}

void alias_calculate_bounds(struct alias* alias, uint32_t x, uint32_t y) {
  image_calculate_bounds(&alias->image, x, y);
}

bool alias_parse_sub_domain(struct alias* alias, FILE* rsp, struct token property, char* message) {
  struct key_value_pair key_value_pair = get_key_value_pair(property.text, '.');
  if (key_value_pair.key && key_value_pair.value) {
    struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
    struct token entry = { key_value_pair.value, strlen(key_value_pair.value) };
    if (token_equals(subdom, SUB_DOMAIN_SHADOW))
      return shadow_parse_sub_domain(&alias->image.shadow, rsp, entry, message);
    else {
      fprintf(rsp, "Invalid subdomain: %s \n", subdom.text);
      printf("Invalid subdomain: %s \n", subdom.text);
    }
  }
  else if (token_equals(property, PROPERTY_COLOR)) {
    alias->color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
    alias->color_override = true;
    return true;
  } else {
    fprintf(rsp, "Unknown property: %s \n", property.text);
    printf("Unknown property: %s \n", property.text);
  }
  return false;
}

