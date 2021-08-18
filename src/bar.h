#ifndef BAR_H
#define BAR_H

extern CGError SLSDisableUpdate(int cid);
extern CGError SLSReenableUpdate(int cid);
extern CGError SLSNewWindow(int cid, int type, float x, float y, CFTypeRef region, uint32_t *wid);
extern CGError SLSReleaseWindow(int cid, uint32_t wid);
extern CGError SLSSetWindowTags(int cid, uint32_t wid, uint32_t tags[2], int tag_size);
extern CGError SLSClearWindowTags(int cid, uint32_t wid, uint32_t tags[2], int tag_size);
extern CGError SLSSetWindowShape(int cid, uint32_t wid, float x_offset, float y_offset, CFTypeRef shape);
extern CGError SLSSetWindowResolution(int cid, uint32_t wid, double res);
extern CGError SLSSetWindowOpacity(int cid, uint32_t wid, bool isOpaque);
extern CGError SLSSetMouseEventEnableFlags(int cid, uint32_t wid, bool shouldEnable);
extern CGError SLSOrderWindow(int cid, uint32_t wid, int mode, uint32_t relativeToWID);
extern CGError SLSSetWindowLevel(int cid, uint32_t wid, int level);
extern CGContextRef SLWindowContextCreate(int cid, uint32_t wid, CFDictionaryRef options);
extern CGError CGSNewRegionWithRect(CGRect *rect, CFTypeRef *outRegion);

#define kCGSModalWindowTagBit           (1 << 31)
#define kCGSDisableShadowTagBit         (1 <<  3)
#define kCGSHighQualityResamplingTagBit (1 <<  4)
#define kCGSIgnoreForExposeTagBit       (1 <<  7)
#define kCGSStickyTagBit                (1 << 11)

#define ALIGN_NONE   0
#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2
#define ALIGN_TOP    3
#define ALIGN_BOTTOM 4
#define ALIGN_CENTER 5

struct bar_line
{
    CTLineRef line;
    CGFloat ascent;
    CGFloat descent;
    CGRect bounds;
    struct rgba_color color;
};

struct bar
{
    uint32_t id;
    uint32_t did;
    CGContextRef context;
    CFRunLoopTimerRef refresh_timer;
    CFRunLoopTimerRef shell_refresh_timer;
    CGRect frame;
    CGPoint origin;
};

void bar_refresh(struct bar *bar);
void bar_resize(struct bar *bar);
struct bar *bar_create(uint32_t did);
void bar_destroy(struct bar *bar);

#endif
