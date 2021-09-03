#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_EVENT_HANDLER(name) void name(uint32_t did, CGDisplayChangeSummaryFlags flags, void *context)
typedef DISPLAY_EVENT_HANDLER(display_callback);

extern CFUUIDRef CGDisplayCreateUUIDFromDisplayID(uint32_t did);
extern CFArrayRef SLSCopyManagedDisplays(int cid);
extern uint64_t SLSManagedDisplayGetCurrentSpace(int cid, CFStringRef uuid);

extern CFStringRef SLSCopyBestManagedDisplayForRect(int cid, CGRect rect);
extern CGError SLSGetCurrentCursorLocation(int cid, CGPoint *point);
extern CFStringRef SLSCopyActiveMenuBarDisplayIdentifier(int cid);
extern CGError SLSGetMenuBarAutohideEnabled(int cid, int *enabled);
extern CGError SLSGetRevealedMenuBarBounds(CGRect *rect, int cid, uint64_t sid);
extern CFStringRef SLSCopyBestManagedDisplayForPoint(int cid, CGPoint point);

uint32_t display_main_display_id(void);
uint32_t display_active_display_id(void);
uint32_t display_arrangement_display_id(int arrangement);
bool display_menu_bar_visible(void);
CGRect display_menu_bar_rect(uint32_t did);
uint32_t display_active_display_count(void);
bool display_begin(void);
bool display_end(void);

CFStringRef display_uuid(uint32_t did);
CGRect display_bounds(uint32_t did);
uint64_t display_space_id(uint32_t did);
uint64_t *display_space_list(uint32_t did, int *count);
int display_arrangement(uint32_t did);

#endif
