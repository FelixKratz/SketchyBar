#include "window.h"

static CFTypeRef window_create_region(struct window *window, CGRect frame) {
  window->frame = (CGRect) {{0, 0},{frame.size.width, frame.size.height}};
  window->origin = frame.origin;

  CFTypeRef frame_region;
  CGSNewRegionWithRect(&window->frame, &frame_region);
  return frame_region;
}

void window_create(struct window* window, CGRect frame) {
  uint64_t set_tags = kCGSStickyTagBit | kCGSHighQualityResamplingTagBit;
  uint64_t clear_tags = kCGSSuperStickyTagBit;

  CFTypeRef frame_region = window_create_region(window, frame);
  SLSNewWindow(g_connection, 2, window->origin.x, window->origin.y,
                                                  frame_region,
                                                  &window->id      );

  SLSAddActivationRegion(g_connection, window->id, frame_region);
  CFRelease(frame_region);

  SLSSetWindowResolution(g_connection, window->id, 2.0f);
  SLSSetWindowTags(g_connection, window->id, &set_tags, 64);
  SLSClearWindowTags(g_connection, window->id, &clear_tags, 64);
  SLSSetWindowOpacity(g_connection, window->id, 0);
  window->context = SLWindowContextCreate(g_connection, window->id, 0);
  CGContextSetInterpolationQuality(window->context, kCGInterpolationNone);
}

void window_resize(struct window* window, CGRect frame) {
  CFTypeRef frame_region = window_create_region(window, frame);
  SLSDisableUpdate(g_connection);
  SLSOrderWindow(g_connection, window->id, -1, 0);
  SLSSetWindowShape(g_connection,
                    window->id,
                    window->origin.x,
                    window->origin.y,
                    frame_region     );

  SLSClearActivationRegion(g_connection, window->id);
  SLSAddActivationRegion(g_connection, window->id, frame_region);
  SLSRemoveAllTrackingAreas(g_connection, window->id);

  SLSOrderWindow(g_connection, window->id, 1, 0);
  SLSReenableUpdate(g_connection);
  CFRelease(frame_region);
}

void window_close(struct window* window) {
  CGContextRelease(window->context);
  SLSReleaseWindow(g_connection, window->id);

  window->context = NULL;
  window->id = 0;
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
