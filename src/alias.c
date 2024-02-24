#include "alias.h"

void print_all_menu_items(FILE* rsp) {
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
  if (__builtin_available(macOS 11.0, *)) {
    if (!CGRequestScreenCaptureAccess()) {
      respond(rsp, "[!] Query (default_menu_items): Screen Recording "
                   "Permissions not given. Restart SketchyBar after granting "
                   "permissions.\n");
      return;
    }
  }

#endif
  CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionAll,
                                                      kCGNullWindowID        );
  int window_count = CFArrayGetCount(window_list);

  float x_pos[window_count];
  char* owner[window_count];
  char* name[window_count];
  memset(owner, 0, sizeof(owner));
  memset(name, 0, sizeof(name));

  int item_count = 0;
  for (int i = 0; i < window_count; ++i) {
    x_pos[i] = -9999.f;
    CFDictionaryRef dictionary = CFArrayGetValueAtIndex(window_list, i);
    if (!dictionary) continue;

    CFStringRef owner_ref = CFDictionaryGetValue(dictionary,
                                                 kCGWindowOwnerName);

    CFNumberRef owner_pid_ref = CFDictionaryGetValue(dictionary,
                                                     kCGWindowOwnerPID);

    CFStringRef name_ref = CFDictionaryGetValue(dictionary, kCGWindowName);
    CFNumberRef layer_ref = CFDictionaryGetValue(dictionary, kCGWindowLayer);
    CFDictionaryRef bounds_ref = CFDictionaryGetValue(dictionary,
                                                      kCGWindowBounds);

    if (!name_ref || !owner_ref || !owner_pid_ref || !layer_ref || !bounds_ref)
      continue;

    long long int layer = 0;
    CFNumberGetValue(layer_ref, CFNumberGetType(layer_ref), &layer);
    uint64_t owner_pid = 0;
    CFNumberGetValue(owner_pid_ref,
                     CFNumberGetType(owner_pid_ref),
                     &owner_pid                     );

    if (layer != MENUBAR_LAYER) continue;
    CGRect bounds = CGRectNull;
    if (!CGRectMakeWithDictionaryRepresentation(bounds_ref, &bounds)) continue;
    owner[item_count] = cfstring_copy(owner_ref);
    name[item_count] = cfstring_copy(name_ref);
    x_pos[item_count++] = bounds.origin.x;
  }

  if (item_count > 0) {
    fprintf(rsp, "[\n");
    int counter = 0;
    for (int i = 0; i < item_count; i++) {
      float current_pos = x_pos[0];
      uint32_t current_pos_id = 0;
      for (int j = 0; j < window_count; j++) {
        if (!name[j] || !owner[j]) continue;
        if (x_pos[j] > current_pos) {
          current_pos = x_pos[j];
          current_pos_id = j;
        }
      }

      if (!name[current_pos_id] || !owner[current_pos_id]) continue;
      if (strcmp(name[current_pos_id], "") != 0) {
        if (counter++ > 0) {
          fprintf(rsp, ", \n");
        }
        fprintf(rsp, "\t\"%s,%s\"", owner[current_pos_id],
                                    name[current_pos_id]  );
      }
      x_pos[current_pos_id] = -9999.f;
    }
    fprintf(rsp, "\n]\n");
    for (int i = 0; i < window_count; i++) {
      if (owner[i]) free(owner[i]);
      if (name[i]) free(name[i]);
    }
  }
  CFRelease(window_list);
}

void alias_get_permission(struct alias* alias) { 
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000 
  if (__builtin_available(macOS 11.0, *)) {
    alias->permission = CGRequestScreenCaptureAccess();
  }
#else
  if(__builtin_available(macos 10.15, *)) {
    CGImageRef img = CGWindowListCreateImage(CGRectMake(0, 0, 1, 1),
                                             kCGWindowListOptionOnScreenOnly,
                                             kCGNullWindowID,
                                             kCGWindowImageDefault           );

    CFRelease(img);
  }
#endif
}

void alias_init(struct alias* alias) {
  alias->name = NULL;
  alias->owner = NULL;
  alias->color_override = false;
  color_init(&alias->color, 0xffff0000);
  alias->update_frequency = 1;
  alias->counter = 0;

  window_init(&alias->window);
  image_init(&alias->image);
}

static void alias_find_window(struct alias* alias) {
  CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionAll,
                                                      kCGNullWindowID        );
  int window_count = CFArrayGetCount(window_list);

  for (int i = 0; i < window_count; ++i) {
    CFDictionaryRef dictionary = CFArrayGetValueAtIndex(window_list, i);
    if (!dictionary) continue;

    CFStringRef owner_ref = CFDictionaryGetValue(dictionary,
                                                 kCGWindowOwnerName);

    CFNumberRef owner_pid_ref = CFDictionaryGetValue(dictionary,
                                                     kCGWindowOwnerPID);

    CFStringRef name_ref = CFDictionaryGetValue(dictionary, kCGWindowName);
    if (!name_ref) continue;
    if (!owner_ref) continue;
    char* owner = cfstring_copy(owner_ref);
    char* name = cfstring_copy(name_ref);

    if (!(alias->owner && strcmp(alias->owner, owner) == 0
          && ((alias->name && strcmp(alias->name, name) == 0)
              || (!alias->name && strcmp(name, "") != 0)     ))) {
      free(owner);
      free(name);
      continue;
    }
    free(owner);
    free(name);

    CFNumberRef layer_ref = CFDictionaryGetValue(dictionary, kCGWindowLayer);
    if (!layer_ref) continue;

    uint64_t layer = 0;
    CFNumberGetValue(layer_ref, CFNumberGetType(layer_ref), &layer);
    if (layer != MENUBAR_LAYER) continue;

    CFNumberGetValue(owner_pid_ref,
                     CFNumberGetType(owner_pid_ref),
                     &alias->pid                    );

    CFNumberRef window_id_ref = CFDictionaryGetValue(dictionary,
                                                     kCGWindowNumber);

    if (!window_id_ref) continue;
    CFDictionaryRef bounds_ref = CFDictionaryGetValue(dictionary, kCGWindowBounds);
    if (!bounds_ref) continue;

    CGRect bounds;
    CGRectMakeWithDictionaryRepresentation(bounds_ref, &bounds);

    uint64_t wid;
    CFNumberGetValue(window_id_ref,
                     CFNumberGetType(window_id_ref),
                     &wid                           );

    alias->window.id = (uint32_t)wid;
    alias->window.frame.size = bounds.size;
    alias->window.origin = bounds.origin;

    CFRelease(window_list);
    return;
  }
  alias->window.id = 0;
  CFRelease(window_list);
}

static bool alias_update_image(struct alias* alias, bool forced) {
  if (alias->window.id == 0) alias_find_window(alias);
  if (alias->window.id == 0) return false;

  bool disabled = false;
  CGImageRef image_ref = window_capture(&alias->window, &disabled);

  if (!image_ref) {
    if (!disabled) {
      alias->window.id = 0;
      image_destroy(&alias->image);
    }
    return false;
  }

  return image_set_image(&alias->image,
                         image_ref,
                         alias->window.frame,
                         forced              );
}

void alias_setup(struct alias* alias, char* owner, char* name) {
  alias->name = name;
  alias->owner = owner;
  alias_get_permission(alias);
  alias_update_image(alias, true);
}

uint32_t alias_get_length(struct alias* alias) {
  if (alias->image.image_ref) return alias->image.bounds.size.width;
  return 0;
}

uint32_t alias_get_height(struct alias* alias) {
  if (alias->image.image_ref) return alias->image.bounds.size.height;
  return 0;
}

bool alias_update(struct alias* alias, bool forced) {
  if (alias->update_frequency == 0) return false;

  alias->counter++;
  if (forced || alias->counter >= alias->update_frequency) {
    alias->counter = 0;
    if (alias_update_image(alias, forced)) {
      return true;
    }
  }
  return false;
}

void alias_draw(struct alias* alias, CGContextRef context) {
  if (alias->color_override) {
    CGContextSaveGState(context);
    image_draw(&alias->image, context);
    CGContextClipToMask(context, alias->image.bounds, alias->image.image_ref);
    CGContextSetRGBFillColor(context,
                             alias->color.r,
                             alias->color.g,
                             alias->color.b,
                             alias->color.a );

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
  struct key_value_pair key_value_pair = get_key_value_pair(property.text,'.');
  if (key_value_pair.key && key_value_pair.value) {
    struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
    struct token entry = { key_value_pair.value, strlen(key_value_pair.value)};
    if (token_equals(subdom, SUB_DOMAIN_SHADOW))
      return shadow_parse_sub_domain(&alias->image.shadow,
                                     rsp,
                                     entry,
                                     message              );
    else if (token_equals(subdom, SUB_DOMAIN_COLOR)) {
      bool changed = !alias->color_override;
      alias->color_override = true;
      return color_parse_sub_domain(&alias->color, rsp, entry, message)
            || changed;
    }
    else {
      respond(rsp, "[!] Alias: Invalid subdomain '%s'\n", subdom.text);
    }
  }
  else if (token_equals(property, PROPERTY_COLOR)) {
    color_set_hex(&alias->color, token_to_uint32t(get_token(&message)));
    alias->color_override = true;
    return true;
  } else if (token_equals(property, PROPERTY_SCALE)) {
    return image_set_scale(&alias->image, token_to_float(get_token(&message)));
  } else if (token_equals(property, PROPERTY_UPDATE_FREQ)) {
    alias->update_frequency = token_to_uint32t(get_token(&message));
    return false;
  } else {
    respond(rsp, "[!] Alias: Invalid property '%s' \n", property.text);
  }
  return false;
}
