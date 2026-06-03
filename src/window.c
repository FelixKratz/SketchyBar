#include "window.h"
#include "bar_manager.h"
#include "misc/helpers.h"
#include "surface.h"
#include "layer.h"
#include <pthread.h>

extern struct bar_manager g_bar_manager;
extern int64_t g_disable_capture;
extern CGError (* SBSLSTransactionAddPostDecodeAction)(CFTypeRef, void (^)());

int g_space = 0;

void window_init(struct window* window) {
  window->context = NULL;
  window->surface = NULL;
  window->parent = NULL;
  window->frame = CGRectNull;
  window->id = 0;
  window->origin = CGPointZero;
  window->needs_move = false;
  window->needs_resize = false;
  window->order_mode = W_ABOVE;
  window->refc = 1;
}

static CFTypeRef window_create_region(struct window* window, CGRect frame) {
  CFTypeRef frame_region;
  CGSNewRegionWithRect(&frame, &frame_region);
  return frame_region;
}

struct window* window_create() {
  struct window* window = malloc(sizeof(struct window));
  memset(window, 0, sizeof(struct window));
  window_init(window);
  return window;
}

void window_destroy(struct window* window) {
  if (--window->refc > 0) return;
  window_close(window);
  free(window);
}

static void window_clear_background(struct window* window) {
  if (window->context) {
    CGContextClearRect(window->context, window->frame);
    CGContextFlush(window->context);
  }
}

void window_open(struct window* window, CGRect frame) {
  uint64_t set_tags = kCGSExposeFadeTagBit | kCGSPreventsActivationTagBit;
  uint64_t clear_tags = 0;

  window->origin = frame.origin;
  window->frame.origin = CGPointZero;
  window->frame.size = frame.size;
  frame.origin = CGPointZero;

  uint32_t id;
  CFTypeRef frame_region = window_create_region(window, frame);
  CFTypeRef empty_region = CGRegionCreateEmptyRegion();
  SLSNewWindowWithOpaqueShapeAndContext(g_connection,
                                        kCGBackingStoreBuffered,
                                        frame_region,
                                        empty_region,
                                        13 | (1 << 18),
                                        &set_tags,
                                        window->origin.x,
                                        window->origin.y,
                                        64,
                                        &id,
                                        NULL                    );
  CFRelease(empty_region);
  CFRelease(frame_region);

  window->id = id;

  SLSSetWindowResolution(g_connection, window->id, 2.0f);
  SLSSetWindowTags(g_connection, window->id, &set_tags, 64);
  SLSClearWindowTags(g_connection, window->id, &clear_tags, 64);
  SLSSetWindowOpacity(g_connection, window->id, 0);

  window->context = SLWindowContextCreate(g_connection, window->id, NULL);
  window_clear_background(window);

  CGContextSetInterpolationQuality(window->context, kCGInterpolationNone);
  window->surface = surface_create(window);
  window->needs_move = false;
  window->needs_resize = false;


  if (g_bar_manager.sticky) {
    if (!g_space) {
      g_space = SLSSpaceCreate(g_connection, 1, 0);
      SLSSpaceSetAbsoluteLevel(g_connection, g_space, 0);

      CFArrayRef space_list = cfarray_of_cfnumbers(&g_space,
                                                   sizeof(uint32_t),
                                                   1,
                                                   kCFNumberSInt32Type);
      SLSShowSpaces(g_connection, space_list);
      CFRelease(space_list);
    }

    CFArrayRef window_list = cfarray_of_cfnumbers(&window->id,
                                                  sizeof(uint32_t),
                                                  1,
                                                  kCFNumberSInt32Type);

    SLSSpaceAddWindowsAndRemoveFromSpaces(g_connection,
                                          g_space,
                                          window_list,
                                          0x7          );

    CFRelease(window_list);
  }
}

void window_clear(struct window* window) {
  window->context = NULL;
  window->surface = NULL;
  window->parent = NULL;
  window->id = 0;
  window->origin = CGPointZero;
  window->frame = CGRectNull;
  window->needs_move = false;
  window->needs_resize = false;
  window->refc = 1;
}

void window_flush(struct window* window) {
  surface_flush(window->surface);
}

void windows_freeze() {
  if (g_transaction) return;

  if (__builtin_available(macOS 26.0, *)) { }
  else SLSDisableUpdate(g_connection);

  g_transaction = SLSTransactionCreate(g_connection);
}

void windows_unfreeze() {
  if (g_transaction) {
    SLSTransactionCommit(g_transaction, 0x0);
    CFRelease(g_transaction);
    g_transaction = NULL;

    if (__builtin_available(macOS 26.0, *)) { }
    else SLSReenableUpdate(g_connection);
  }
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

void window_move(struct window* window, CGPoint point) {
  window->origin = point;

  if (__builtin_available(macOS 12.0, *)) {
    // Monterey and later
    windows_freeze();
    SLSTransactionMoveWindowWithGroup(g_transaction, window->id, point);
  } else {
    // Big Sur and previous
    SLSMoveWindow(g_connection, window->id, &point);
    CFNumberRef number = CFNumberCreate(NULL,
                                        kCFNumberSInt32Type,
                                        &window->id         );

    const void* values[1] = { number };
    CFArrayRef array = CFArrayCreate(NULL, values , 1, &kCFTypeArrayCallBacks);
    SLSReassociateWindowsSpacesByGeometry(g_connection, array);
    CFRelease(array);
    CFRelease(number);
  }

}

static void window_defer_update(struct window* window) {
  void (^block)() = ^{
    if (!window->surface) return;
    if (--window->refc <= 0) window_destroy(window);
    else {
      layer_set_bounds(window->surface->layer, window->frame);
      window_flush(window);
    }
  };

  if (pthread_main_np()) dispatch_async(dispatch_get_main_queue(), block);
  else dispatch_sync(dispatch_get_main_queue(), block);
}

bool window_apply_frame(struct window* window, bool forced) {
  if (window->needs_resize || forced) {
    windows_freeze();
    CFTypeRef frame_region = window_create_region(window, window->frame);
    window->refc++;

    if (__builtin_available(macOS 26.0, *)) {
      SLSTransactionSetWindowShape(g_transaction, window->id,
                                                  window->origin.x,
                                                  window->origin.y,
                                                  frame_region);
      window_move(window, window->origin);
      if (SBSLSTransactionAddPostDecodeAction) {
        SBSLSTransactionAddPostDecodeAction(g_transaction, ^{
          window_defer_update(window);
        });
      } else window_defer_update(window);
    }
    else if (__builtin_available(macOS 13.0, *)) {
      // Ventura and later
      SLSSetWindowShape(g_connection, window->id,
                                      g_nirvana.x,
                                      g_nirvana.y,
                                      frame_region);
      window_clear_background(window);
      window_move(window, window->origin);
      window_defer_update(window);
    } else {
      // Monterey and previous
      if (window->parent) {
        SLSOrderWindow(g_connection, window->id, 0, window->parent->id);
      }

      SLSSetWindowShape(g_connection, window->id, 0, 0, frame_region);
      window_clear_background(window);

      if (window->parent) {
        window_order(window, window->parent, window->order_mode);
      }
      window_move(window, window->origin);
      window_defer_update(window);
    }

    surface_resize(window->surface, window);
    CFRelease(frame_region);

    window->needs_move = false;
    window->needs_resize = false;
    return true;
  } else if (window->needs_move) {
    window_move(window, window->origin);
    window->needs_move = false;
    return false;
  }
  return false;
}

void window_send_to_space(struct window* window, uint64_t dsid) {
  CFArrayRef window_list = cfarray_of_cfnumbers(&window->id,
                                                sizeof(uint32_t),
                                                1,
                                                kCFNumberSInt32Type);

  SLSMoveWindowsToManagedSpace(g_connection, window_list, dsid);
  if (CGPointEqualToPoint(window->origin, g_nirvana)) {
    SLSMoveWindow(g_connection, window->id, &g_nirvana);
  }
  CFRelease(window_list);
}

void window_close(struct window* window) {
  if (!window->id) return;

  SLSOrderWindow(g_connection, window->id, 0, 0);
  surface_destroy(window->surface);
  if (window->context) CGContextRelease(window->context);
  SLSReleaseWindow(g_connection, window->id);
  window_clear(window);
}

void window_set_level(struct window* window, uint32_t level) {
  windows_freeze();
  if (__builtin_available(macOS 14.0, *)) {
    // Sonoma and later
    SLSTransactionSetWindowLevel(g_transaction, window->id, level);
  } else {
    // Ventura and previous
    SLSSetWindowLevel(g_connection, window->id, level);
  }
}

void window_order(struct window* window, struct window* parent, int mode) {
  windows_freeze();
  window->parent = parent;
  if (mode != W_OUT) window->order_mode = mode;

  if (__builtin_available(macOS 14.0, *)) {
    // Sonoma and later
    SLSTransactionOrderWindow(g_transaction,
                              window->id,
                              mode,
                              parent ? parent->id : 0);
  } else {
    // Ventura and previous
    SLSOrderWindow(g_connection, window->id, mode, parent ? parent->id : 0);
  }
}

void window_assign_mouse_tracking_area(struct window* window, CGRect rect) {
  SLSRemoveAllTrackingAreas(g_connection, window->id);
  SLSAddTrackingRect(g_connection, window->id, rect);
}

void window_set_blur_radius(struct window* window, uint32_t blur_radius) {
  SLSSetWindowBackgroundBlurRadius(g_connection, window->id, blur_radius);
  if (window->context) window_clear_background(window);
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

CGImageRef window_capture(struct window* window, bool* disabled) {
  if (g_disable_capture) {
    int64_t time = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW_APPROX);
    if (g_disable_capture < 0) {
      *disabled = true;
      return NULL;
    } else if (time - g_disable_capture > (1ULL << 30)) {
      g_disable_capture = 0;
    } else {
      *disabled = true;
      return NULL;
    }
  }

  *disabled = false;
  CGImageRef image_ref = NULL;

  uint64_t wid = window->id;
  SLSCaptureWindowsContentsToRectWithOptions(g_connection,
                                             &wid,
                                             true,
                                             CGRectNull,
                                             1 << 8,
                                             &image_ref  );

  CGRect bounds;
  SLSGetScreenRectForWindow(g_connection, wid, &bounds);
  bounds.size.width = (uint32_t) (bounds.size.width + 0.5);
  window->frame.size = bounds.size;

  return image_ref;
}
