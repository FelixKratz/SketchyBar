#include "window.h"

static CFTypeRef window_create_region(struct window *window, CGRect frame) {
  CFTypeRef frame_region;
  CGSNewRegionWithRect(&frame, &frame_region);
  return frame_region;
}

void window_create(struct window* window, CGRect frame) {
  uint64_t set_tags = kCGSStickyTagBit | kCGSHighQualityResamplingTagBit;
  uint64_t clear_tags = kCGSSuperStickyTagBit;

  window->origin = frame.origin;
  window->frame.origin = (CGPoint){0, 0};
  window->frame.size = frame.size;

  frame.origin = (CGPoint){0, 0};
  CFTypeRef frame_region = window_create_region(window, frame);
  uint64_t id;
  SLSNewWindow(g_connection, 2, window->origin.x, window->origin.y,
                                                  frame_region,
                                                  &id              );

  window->id = (uint32_t)id;

  CFRelease(frame_region);

  SLSSetWindowResolution(g_connection, window->id, 2.0f);
  SLSSetWindowTags(g_connection, window->id, &set_tags, 64);
  SLSClearWindowTags(g_connection, window->id, &clear_tags, 64);
  SLSSetWindowOpacity(g_connection, window->id, 0);

  const void* keys[] = { CFSTR("CGWindowContextShouldUseCA") };
  const void* values[] = { kCFBooleanTrue };
  CFDictionaryRef dict = CFDictionaryCreate(NULL,
                                            keys,
                                            values,
                                            1,
                                            &kCFTypeDictionaryKeyCallBacks,
                                            &kCFTypeDictionaryValueCallBacks);

  window->context = SLWindowContextCreate(g_connection, window->id, dict);
  CFRelease(dict);

  CGContextSetInterpolationQuality(window->context, kCGInterpolationNone);
  window->needs_move = false;
  window->needs_resize = false;
}

void windows_freeze() {
  SLSDisableUpdate(g_connection);
}

void windows_unfreeze() {
  SLSReenableUpdate(g_connection);
}

void window_set_frame(struct window* window, CGRect frame) {
  if (window->needs_move
      || !CGPointEqualToPoint(window->origin, frame.origin)) {

    window->needs_move = true;
    window->origin = frame.origin;
  }

  if (window->needs_resize
      || !CGSizeEqualToSize(window->frame.size, frame.size)) {

    window->needs_resize = true;
    window->frame.size = frame.size;
  }
}

bool window_apply_frame(struct window* window) {
  if (window->needs_resize) {
    CFTypeRef frame_region = window_create_region(window, window->frame);
    SLSSetWindowShape(g_connection,
                      window->id,
                      window->origin.x,
                      window->origin.y,
                      frame_region     );

    CFRelease(frame_region);
    window->needs_move = false;
    window->needs_resize = false;
    return true;
  } else if (window->needs_move) {
    CGPoint origin = window->origin;
    SLSMoveWindow(g_connection, window->id, &origin);
    window->needs_move = false;
    return false;
  }
  return false;
}

void window_close(struct window* window) {
  CGContextRelease(window->context);
  SLSReleaseWindow(g_connection, window->id);

  window->context = NULL;
  window->id = 0;
  window->origin = CGPointZero;
  window->frame = CGRectNull;
  window->needs_move = false;
  window->needs_resize = false;
}

void window_set_level(struct window* window, uint32_t level) {
  SLSSetWindowLevel(g_connection, window->id, level);
}

void window_set_blur_radius(struct window* window, uint32_t blur_radius) {
  SLSSetWindowBackgroundBlurRadius(g_connection, window->id, blur_radius);
}

void context_set_font_smoothing(CGContextRef context, bool smoothing) {
  CGContextSetAllowsFontSmoothing(context, smoothing);
}

void window_disable_shadow(struct window* window) {
  CFIndex shadow_density = 0;
  CFNumberRef shadow_density_cf = CFNumberCreate(kCFAllocatorDefault,
                                                 kCFNumberCFIndexType,
                                                 &shadow_density      );

  const void *keys[1] = { CFSTR("com.apple.WindowShadowDensity") };
  const void *values[1] = { shadow_density_cf };
  CFDictionaryRef shadow_props_cf = CFDictionaryCreate(NULL,
                                             keys,
                                             values,
                                             1,
                                             &kCFTypeDictionaryKeyCallBacks,
                                             &kCFTypeDictionaryValueCallBacks);

  SLSWindowSetShadowProperties(window->id, shadow_props_cf);
  CFRelease(shadow_density_cf);
  CFRelease(shadow_props_cf);
}
