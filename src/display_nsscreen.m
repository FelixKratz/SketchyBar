#include "display_nsscreen.h"

#import <AppKit/AppKit.h>

int display_nsscreen_top_inset(uint32_t did) {
  @autoreleasepool {
    for (NSScreen* screen in [NSScreen screens]) {
      NSNumber* screen_number = screen.deviceDescription[@"NSScreenNumber"];
      if (!screen_number || screen_number.unsignedIntValue != did) continue;

      NSRect frame = screen.frame;
      NSRect visible_frame = screen.visibleFrame;
      CGFloat top_inset = NSMaxY(frame) - NSMaxY(visible_frame);

      // visibleFrame includes the full reserved strip above normal app windows.
      // On Tahoe that is one pixel taller than the visually aligned menu-bar area,
      // so subtract one pixel to keep SketchyBar flush with the native menu bar.
      top_inset -= 1;
      if (top_inset < 0) top_inset = 0;
      return (int)(top_inset + 0.5);
    }
  }

  return -1;
}
