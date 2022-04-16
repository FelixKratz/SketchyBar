#pragma once
#include "misc/helpers.h"

extern CGError SLSDisableUpdate(int cid);
extern CGError SLSReenableUpdate(int cid);
extern CGError SLSNewWindow(int cid, int type, float x, float y, CFTypeRef region, uint32_t *wid);
extern CGError SLSReleaseWindow(int cid, uint32_t wid);
extern CGError SLSSetWindowTags(int cid, uint32_t wid, uint64_t* tags, int tag_size);
extern CGError SLSClearWindowTags(int cid, uint32_t wid, uint64_t* tags, int tag_size);
extern CGError SLSSetWindowShape(int cid, uint32_t wid, float x_offset, float y_offset, CFTypeRef shape);
extern CGError SLSSetWindowResolution(int cid, uint32_t wid, double res);
extern CGError SLSSetWindowOpacity(int cid, uint32_t wid, bool isOpaque);
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
extern int SLSSpaceGetType(int cid, uint64_t sid);

#define kCGSHighQualityResamplingTagBit (1ULL <<  4)
#define kCGSStickyTagBit                (1ULL << 11)
#define kCGSSuperStickyTagBit           (1ULL << 45)

struct window {
  uint32_t id;
  CGRect frame;
  CGPoint origin;
  CGContextRef context;
};

void window_create(struct window* window, CGRect frame);
void window_close(struct window* window);
void window_resize(struct window* window, CGRect frame);

void window_set_blur_radius(struct window* window, uint32_t blur_radius);
void window_disable_shadow(struct window* window);
void window_set_level(struct window* window, uint32_t level);

void context_set_font_smoothing(CGContextRef context, bool smoothing);
