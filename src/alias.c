#include "alias.h"
#include "menubar_agent.h"
#include "misc/helpers.h"
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CoreFoundation.h>

struct menu_item {
  char* owner;
  char* name;

  float x_pos;
  uint32_t wid;
  uint64_t layer;

  CGRect bounds;
  // Part of the window that shows the item, empty if the item has a window of
  // its own (macOS < 27)
  CGRect crop;
};

void menu_item_init(struct menu_item* menu_item) {
  memset(menu_item, 0, sizeof(struct menu_item)); 
  menu_item->x_pos = g_nirvana.x;
}

void menu_item_clear(struct menu_item* menu_item) {
  if (menu_item->name) free(menu_item->name);
  if (menu_item->owner) free(menu_item->owner);
  memset(menu_item, 0, sizeof(struct menu_item)); 
}

struct menu_item_list {
  struct menu_item* menu_items;
  uint32_t menu_item_count;
};

void menu_item_list_init(struct menu_item_list* menu_item_list, uint32_t menu_item_count) {
  menu_item_list->menu_items = malloc(sizeof(struct menu_item)*menu_item_count);
  menu_item_list->menu_item_count = menu_item_count;
  for (int i = 0; i < menu_item_count; i++) {
    menu_item_init(menu_item_list->menu_items + i);
  }
}

void menu_item_list_sort(struct menu_item_list* menu_item_list) {
  struct menu_item tmp;
  for (int i = 0; i < menu_item_list->menu_item_count; i++) {
    float highest_value = g_nirvana.x;
    uint32_t highest_value_index = 0;
    for (int j = i; j < menu_item_list->menu_item_count; j++) {
      if (menu_item_list->menu_items[j].x_pos > highest_value) {
        highest_value = menu_item_list->menu_items[j].x_pos;
        highest_value_index = j;
      }
    }
    if (i != highest_value_index) {
      tmp = menu_item_list->menu_items[i];
      menu_item_list->menu_items[i]
                            = menu_item_list->menu_items[highest_value_index];
      menu_item_list->menu_items[highest_value_index] = tmp;
    }
  }
}

void menu_item_list_clear(struct menu_item_list* menu_item_list) {
  if (menu_item_list->menu_items) {
    for (int i = 0; i < menu_item_list->menu_item_count; i++) {
      menu_item_clear(menu_item_list->menu_items + i);
    }
    free(menu_item_list->menu_items);
  }
  memset(menu_item_list, 0, sizeof(struct menu_item_list)); 
}

static struct menu_item_list get_menubar_agent_item_list() {
  struct menubar_agent_item* items = NULL;
  uint32_t count = menubar_agent_copy_items(&items);

  struct menu_item_list menu_item_list;
  menu_item_list_init(&menu_item_list, count);
  for (uint32_t i = 0; i < count; i++) {
    struct menu_item* menu_item = menu_item_list.menu_items + i;
    menu_item->owner = items[i].owner;
    menu_item->name = items[i].name;
    menu_item->x_pos = items[i].frame.origin.x;
    menu_item->wid = items[i].wid;
    menu_item->layer = 0;
    menu_item->bounds = items[i].frame;
    menu_item->crop = items[i].crop;
    // Ownership of the strings moved to the menu item
    items[i].owner = NULL;
    items[i].name = NULL;
  }
  menubar_agent_items_free(items, count);
  return menu_item_list;
}

struct menu_item_list get_menu_item_list() {
  // On macOS 27 status items no longer have windows of their own
  if (menubar_agent_available()) return get_menubar_agent_item_list();

  CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionAll,
                                                      kCGNullWindowID        );
  int window_count = CFArrayGetCount(window_list);

  struct menu_item_list menu_item_list;
  menu_item_list_init(&menu_item_list, window_count);

  int item_count = 0;
  for (int i = 0; i < window_count; ++i) {
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
    CFNumberRef window_id_ref = CFDictionaryGetValue(dictionary,
                                                     kCGWindowNumber);


    if (!name_ref || !owner_ref || !owner_pid_ref || !layer_ref || !bounds_ref
        || !window_id_ref)
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
    char* owner_copy = cfstring_copy(owner_ref);
    if (string_equals(owner_copy, "Window Server")) {
      free(owner_copy);
      continue;
    }

    uint64_t wid;
    CFNumberGetValue(window_id_ref,
                     CFNumberGetType(window_id_ref),
                     &wid                           );
    
    struct menu_item* menu_item = menu_item_list.menu_items + item_count++;
    menu_item->owner = owner_copy;
    menu_item->name = cfstring_copy(name_ref);
    menu_item->x_pos = bounds.origin.x;
    menu_item->wid = wid;
    menu_item->layer = layer;
    menu_item->bounds = bounds;
  }

  menu_item_list.menu_item_count = item_count;
  menu_item_list_sort(&menu_item_list);
  CFRelease(window_list);
  return menu_item_list;
}

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
  if (menubar_agent_available() && !menubar_agent_trusted(false)) {
    respond(rsp, "[!] Query (default_menu_items): Accessibility "
                 "Permissions not given. They are needed to find menu bar "
                 "items on this version of macOS.\n");
    return;
  }

  struct menu_item_list menu_item_list = get_menu_item_list();
  if (menu_item_list.menu_item_count > 0) {
    fprintf(rsp, "[\n");
    int counter = 0;
    for (int i = 0; i < menu_item_list.menu_item_count; i++) {
      struct menu_item* item = menu_item_list.menu_items + i;
      if (counter++ > 0) {
        fprintf(rsp, ", \n");
      }
      char* escaped_owner = escape_string(item->owner);
      char* escaped_name = escape_string(item->name);
      fprintf(rsp, "\t\"%s,%s(%d)\"", escaped_owner ? escaped_owner : "(null)",
                                      escaped_name ? escaped_name : "(null)",
                                      counter     );
      free(escaped_owner);
      free(escaped_name);
    }
    fprintf(rsp, "\n]\n");
  }
  menu_item_list_clear(&menu_item_list);
}

void alias_get_permission(struct alias* alias) { 
  alias->menubar_agent = menubar_agent_available();
  if (alias->menubar_agent) menubar_agent_trusted(true);

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
  alias->menubar_agent = false;
  alias->crop = CGRectNull;
  alias->item_frame = CGRectNull;

  window_init(&alias->window);
  image_init(&alias->image);
}

static void alias_find_window(struct alias* alias) {
  struct menu_item_list menu_item_list = get_menu_item_list();
  
  for (int i = 0; i < menu_item_list.menu_item_count; ++i) {
    struct menu_item* item = menu_item_list.menu_items + i;
    if (!alias->owner || strcmp(alias->owner, item->owner) != 0) continue;
    if (!alias->name || strcmp(alias->name, item->name) != 0) {
      if (alias->name) {
        char indexed_name[strlen(alias->name) + 32];
        snprintf(indexed_name, strlen(alias->name) + 32,
                               "%s(%d)",
                               item->name,
                               i + 1                    );
        if (strcmp(alias->name, indexed_name) != 0) continue; 
      } else if (strcmp(item->name, "") != 0 && !alias->menubar_agent) {
        continue;
      }
    }

    alias->window.id = item->wid;
    alias->crop = item->crop;
    alias->item_frame = item->bounds;
    alias->window.frame.size = item->bounds.size;
    alias->window.origin = item->bounds.origin;

    menu_item_list_clear(&menu_item_list);
    return;
  }
  alias->window.id = 0;
  menu_item_list_clear(&menu_item_list);
}

// Copies the part of a captured window that shows the menu bar item. The
// pixels are copied, such that the image data only changes when the item does.
static CGImageRef alias_crop_image(CGImageRef image_ref, CGRect window_frame, CGRect crop) {
  if (window_frame.size.width <= 0) return NULL;
  CGFloat scale = CGImageGetWidth(image_ref) / window_frame.size.width;
  CGRect pixel_crop = CGRectIntegral(CGRectMake(crop.origin.x * scale,
                                                crop.origin.y * scale,
                                                crop.size.width * scale,
                                                crop.size.height * scale));

  CGImageRef cropped_ref = CGImageCreateWithImageInRect(image_ref, pixel_crop);
  if (!cropped_ref) return NULL;

  size_t width = CGImageGetWidth(cropped_ref);
  size_t height = CGImageGetHeight(cropped_ref);
  CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(NULL,
                                               width,
                                               height,
                                               8,
                                               0,
                                               color_space,
                                               kCGImageAlphaPremultipliedFirst
                                               | kCGBitmapByteOrder32Host     );
  CGColorSpaceRelease(color_space);
  if (!context) {
    CGImageRelease(cropped_ref);
    return NULL;
  }

  CGContextDrawImage(context, CGRectMake(0, 0, width, height), cropped_ref);
  CGImageRelease(cropped_ref);
  CGImageRef result = CGBitmapContextCreateImage(context);
  CGContextRelease(context);
  return result;
}

static bool alias_update_image(struct alias* alias, bool forced) {
  // Items drawn by MenuBarAgent move whenever an item next to them changes
  // its size, so their position is looked up on every update.
  if (alias->window.id == 0 || alias->menubar_agent) alias_find_window(alias);
  if (alias->window.id == 0) {
    // The item left the bar, e.g. because its app quit or was hidden
    bool had_image = alias->image.image_ref != NULL;
    if (alias->menubar_agent) image_destroy(&alias->image);
    return alias->menubar_agent && had_image;
  }

  bool disabled = false;
  CGImageRef image_ref = window_capture(&alias->window, &disabled);

  if (!image_ref) {
    if (!disabled) {
      alias->window.id = 0;
      image_destroy(&alias->image);
    }
    return false;
  }

  CGRect frame = alias->window.frame;
  if (alias->menubar_agent && !CGRectIsNull(alias->crop)) {
    CGImageRef cropped_ref = alias_crop_image(image_ref,
                                              alias->window.frame,
                                              alias->crop         );
    CGImageRelease(image_ref);
    if (!cropped_ref) return false;
    image_ref = cropped_ref;
    frame = (CGRect){{0, 0}, alias->crop.size};
  }

  return image_set_image(&alias->image,
                         image_ref,
                         frame,
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

// Forwards a click on the alias to the menu bar item it shows. Only items
// drawn by MenuBarAgent can be clicked this way.
bool alias_click(struct alias* alias) {
  if (!alias->menubar_agent || CGRectIsNull(alias->item_frame)) return false;
  return menubar_agent_press(alias->item_frame);
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
