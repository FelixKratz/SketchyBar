#include "ax.h"

void ax_init(struct ax* ax) {
  ax->element = NULL;

  const void *keys[] = { kAXTrustedCheckOptionPrompt };
  const void *values[] = { kCFBooleanTrue };

  CFDictionaryRef options;
  options = CFDictionaryCreate(kCFAllocatorDefault,
                               keys, values, sizeof(keys) / sizeof(*keys),
                               &kCFCopyStringDictionaryKeyCallBacks,
                               &kCFTypeDictionaryValueCallBacks           );

  ax->is_privileged = AXIsProcessTrustedWithOptions(options);
  CFRelease(options);
}

void ax_perform_click(struct ax* ax) {
  if (!ax->element) return;

  AXUIElementPerformAction(ax->element, kAXPressAction);
}

void ax_get_extra_menu_item(struct ax* ax, pid_t pid, char* name) {
  AXUIElementRef menu_extras_ref = NULL;
  CFArrayRef children_ref = NULL;

  ax->element = AXUIElementCreateApplication(pid);
  AXError error = AXUIElementCopyAttributeValue(ax->element, kAXExtrasMenuBarAttribute, (CFTypeRef*)&menu_extras_ref);
  if (ax->element) CFRelease(ax->element);
  ax->element = NULL;

  if (error == kAXErrorSuccess) {
    error = AXUIElementCopyAttributeValue(menu_extras_ref, kAXVisibleChildrenAttribute, (CFTypeRef*)&children_ref);
    uint32_t count = CFArrayGetCount(children_ref);

    if (count == 1 || (count > 0 && !name)) {
      ax->element = CFArrayGetValueAtIndex(children_ref, 0);
    } else {
      // Loop through array and find correct item via name...
    }
  }

  CFRelease(menu_extras_ref);
  CFRelease(children_ref);
}

void ax_clear(struct ax* ax) {
  if (ax->element) CFRelease(ax->element);
  ax->element = NULL;
}
