#include "window.h"
#include "alias.h"

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

  // const void* keys[] = { CFSTR("CGWindowContextShouldUseCA") };
  // const void* values[] = { kCFBooleanTrue };
  // CFDictionaryRef dict = CFDictionaryCreate(NULL, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  // CGContextRef context = SLWindowContextCreate(g_connection, window->id, dict);
  window->context = SLWindowContextCreate(g_connection, window->id, 0);
  // CFRelease(dict);

  CGContextSetInterpolationQuality(window->context, kCGInterpolationNone);
  // CGLayerRef layer = CGLayerCreateWithContext(context, window->frame.size, 0);

  // window->context = CGLayerGetContext(layer);


  // SLSAddSurface(g_connection, window->id, &window->surface_id);
  // SLSSetSurfaceBounds(g_connection, window->id, window->surface_id, window->frame);
  // SLSBindSurface(g_connection, window->id, window->surface_id, 0, 0, window->context);
  // SLSOrderSurface(g_connection, window->id, window->surface_id, 0, 1);

}

void windows_freeze() {
  SLSDisableUpdate(g_connection);
}

void windows_unfreeze() {
  SLSReenableUpdate(g_connection);
}

void window_resize(struct window* window, CGRect frame) {
  CGRect out;
  SLSGetScreenRectForWindow(g_connection, window->id, &out);

  if (CGRectEqualToRect(frame, out)) return;
  else if (CGSizeEqualToSize(frame.size, out.size)) {
    window->origin = frame.origin;
    SLSMoveWindow(g_connection, window->id, &window->origin);
    return;
  }
  
  CFTypeRef frame_region = window_create_region(window, frame);
  SLSSetWindowShape(g_connection,
                    window->id,
                    0,
                    0,
                    frame_region     );

  SLSMoveWindow(g_connection, window->id, &window->origin);
  SLSClearActivationRegion(g_connection, window->id);
  SLSAddActivationRegion(g_connection, window->id, frame_region);
  SLSRemoveAllTrackingAreas(g_connection, window->id);

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
