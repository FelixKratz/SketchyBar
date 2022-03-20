#pragma once
#include "background.h"
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

#define kCGSHighQualityResamplingTagBit (1ULL <<  4)
#define kCGSStickyTagBit                (1ULL << 11)
#define kCGSSuperStickyTagBit           (1ULL << 45)

struct bar_item;

struct popup {
  bool drawing;
  bool horizontal;
  bool overrides_cell_size;

  char align;

  uint32_t id;
  uint32_t adid;
  uint32_t cell_size;
  int y_offset;

  CGRect frame;
  CGPoint anchor;
  CGContextRef context;


  struct bar_item** items;
  uint32_t num_items;

  struct background background;
};

void popup_init(struct popup* popup);
void popup_set_anchor(struct popup* popup, CGPoint anchor, uint32_t adid);
void popup_add_item(struct popup* popup, struct bar_item* item);
void popup_set_drawing(struct popup* popup, bool drawing);
void popup_remove_item(struct popup* popup, struct bar_item* bar_item);

void popup_calculate_bounds(struct popup* popup);
void popup_resize(struct popup* popup);
void popup_draw(struct popup* popup);
void popup_destroy(struct popup* popup);

bool popup_parse_sub_domain(struct popup* popup, FILE* rsp, struct token property, char* message);
