#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_EVENT_HANDLER(name) void name(uint32_t did, CGDisplayChangeSummaryFlags flags, void *context)
typedef DISPLAY_EVENT_HANDLER(display_callback);

extern CFUUIDRef CGDisplayCreateUUIDFromDisplayID(uint32_t did);
extern CFArrayRef SLSCopyManagedDisplays(int cid);
extern uint64_t SLSManagedDisplayGetCurrentSpace(int cid, CFStringRef uuid);

CFStringRef display_uuid(uint32_t did);
CGRect display_bounds(uint32_t did);
uint64_t display_space_id(uint32_t did);
uint64_t *display_space_list(uint32_t did, int *count);
int display_arrangement(uint32_t did);

#endif
