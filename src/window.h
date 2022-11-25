#pragma once
#include "misc/helpers.h"

extern CGError SLSDisableUpdate(int cid);
extern CGError SLSReenableUpdate(int cid);
extern CGError SLSNewWindow(int cid, int type, float x, float y, CFTypeRef region, uint64_t *wid);
extern CGError SLSReleaseWindow(int cid, uint32_t wid);
extern CGError SLSSetWindowTags(int cid, uint32_t wid, uint64_t* tags, int tag_size);
extern CGError SLSClearWindowTags(int cid, uint32_t wid, uint64_t* tags, int tag_size);
extern CGError SLSSetWindowShape(int cid, uint32_t wid, float x_offset, float y_offset, CFTypeRef shape);
extern CGError SLSSetWindowResolution(int cid, uint32_t wid, double res);
extern CGError SLSSetWindowOpacity(int cid, uint32_t wid, bool isOpaque);
extern CGError SLSSetWindowAlpha(int cid, uint32_t wid, float alpha);
extern CGError SLSSetWindowBackgroundBlurRadius(int cid, uint32_t wid, uint32_t radius);
extern CGError SLSOrderWindow(int cid, uint32_t wid, int mode, uint32_t relativeToWID);
extern CGError SLSSetWindowLevel(int cid, uint32_t wid, int level);
extern CGContextRef SLWindowContextCreate(int cid, uint32_t wid, CFDictionaryRef options);
extern CGError CGSNewRegionWithRect(CGRect *rect, CFTypeRef *outRegion);
extern CGError SLSAddActivationRegion(uint32_t cid, uint32_t wid, CFTypeRef region);
extern CGError SLSAddTrackingRect(uint32_t cid, uint32_t wid, CGRect rect);
extern CGError SLSClearActivationRegion(uint32_t cid, uint32_t wid);
extern CGError SLSRemoveAllTrackingAreas(uint32_t cid, uint32_t wid);
extern CGError SLSMoveWindow(int cid, uint32_t wid, CGPoint *point);
extern CGError SLSWindowSetShadowProperties(uint32_t wid, CFDictionaryRef properties);
extern CGError SLSAddWindowToWindowOrderingGroup(int cid, uint32_t parent_wid, uint32_t child_wid, int order);
extern CGError SLSRemoveFromOrderingGroup(int cid, uint32_t wid);
extern CGError SLSReassociateWindowsSpacesByGeometry(int cid, CFArrayRef wids);
extern void SLSMoveWindowsToManagedSpace(int cid, CFArrayRef window_list, uint64_t sid);

extern void SLSCaptureWindowsContentsToRectWithOptions(uint32_t cid, uint64_t* wid, bool meh, CGRect bounds, uint32_t flags, CGImageRef* image);
extern int SLSGetScreenRectForWindow(uint32_t cid, uint32_t wid, CGRect* out);

extern int SLSSpaceGetType(int cid, uint64_t sid);

extern CGError SLSAddSurface(int cid, uint32_t wid, uint32_t* outSID);
extern CGError SLSRemoveSurface(int cid, uint32_t wid, uint32_t sid);
extern CGError SLSBindSurface(int cid, uint32_t wid, uint32_t sid, int param1, int param2, unsigned int context_id);
extern CGError SLSSetSurfaceBounds(int cid, uint32_t wid, uint32_t sid, CGRect bounds);
extern CGError SLSSetSurfaceOpacity(int cid, uint32_t wid, uint32_t sid, bool opaque);
extern CGError SLSOrderSurface(int cid, uint32_t wid, uint32_t surface, int mode, uint32_t other_surface);
extern CGError SLSSetSurfaceResolution(int cid, uint32_t wid, uint32_t sid, CGFloat scale);
extern CGError SLSFlushSurface(int cid, uint32_t wid, uint32_t surface, int param);
extern CGError SLSSetSurfaceColorSpace(int cid, uint32_t wid, uint32_t surface, CGColorSpaceRef color_space);

#define kCGSHighQualityResamplingTagBit (1ULL <<  4)
#define kCGSStickyTagBit                (1ULL << 11)
#define kCGSSuperStickyTagBit           (1ULL << 45)

#define W_ABOVE  1
#define W_OUT    0
#define W_BELOW -1

struct window {
  struct window* parent;
  int order_mode;
  bool needs_move;
  bool needs_resize;

  uint32_t id;
  uint32_t surface_id;

  CGRect frame;
  CGPoint origin;
  CGContextRef context;
};

void window_init(struct window* window);
void window_create(struct window* window, CGRect frame);
void window_close(struct window* window);

void window_move(struct window* window, CGPoint point);
void window_set_frame(struct window* window, CGRect frame);
bool window_apply_frame(struct window* window);
void window_send_to_space(struct window* window, uint64_t dsid);

void window_set_blur_radius(struct window* window, uint32_t blur_radius);
void window_disable_shadow(struct window* window);
void window_set_level(struct window* window, uint32_t level);
void window_order(struct window* window, struct window* parent, int mode);
void window_assign_mouse_tracking_area(struct window* window, CGRect rect);

CGImageRef window_capture(struct window* window);

void context_set_font_smoothing(CGContextRef context, bool smoothing);

void windows_freeze();
void windows_unfreeze();
