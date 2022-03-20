#include "ax.h"

void ax_init(struct ax* ax, bool check_privileges) {
  ax->elements = NULL;
  ax->element_count = 0;
  if (check_privileges) {
    const void *keys[] = { kAXTrustedCheckOptionPrompt };
    const void *values[] = { kCFBooleanTrue };

    CFDictionaryRef options;
    options = CFDictionaryCreate(kCFAllocatorDefault,
        keys, values, sizeof(keys) / sizeof(*keys),
        &kCFCopyStringDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks           );

    ax->is_privileged = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
  } else ax->is_privileged = g_ax.is_privileged;

}

void ax_perform_click(AXUIElementRef element) {
  if (!element) return;

  AXUIElementPerformAction(element, kAXPressAction);
}

AXUIElementRef ax_get_extra_menu_item(struct ax* ax, pid_t pid, char* name) {
  AXUIElementRef extra_menus_ref = NULL;
  AXUIElementRef extra_menu = NULL;
  CFArrayRef children_ref = NULL;

  AXUIElementRef app = AXUIElementCreateApplication(pid);
  AXError error = AXUIElementCopyAttributeValue(app, kAXExtrasMenuBarAttribute, (CFTypeRef*)&extra_menus_ref);
  if (app) CFRelease(app);

  if (error == kAXErrorSuccess) {
    error = AXUIElementCopyAttributeValue(extra_menus_ref, kAXVisibleChildrenAttribute, (CFTypeRef*)&children_ref);
    uint32_t count = CFArrayGetCount(children_ref);

    if (count == 1 || (count > 0 && !name)) {
      extra_menu = CFRetain(CFArrayGetValueAtIndex(children_ref, 0));
    } else {
      // Loop through array and find correct item via name...
    }
    CFRelease(extra_menus_ref);
    CFRelease(children_ref);
  }
  return extra_menu;
}

void ax_clear(struct ax* ax) {
  for (int i = 0; i < ax->element_count; i++) {
    if (ax->elements[i]) CFRelease(ax->elements[i]);
  }
  if (ax->elements) free(ax->elements);
}
