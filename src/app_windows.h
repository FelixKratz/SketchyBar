#pragma once
#include "event.h"

extern CGError SLSGetWindowOwner(int cid, uint32_t wid, int* out_cid);
extern CGError SLSConnectionGetPID(int cid, pid_t *pid);
extern CFArrayRef SLSCopyWindowsWithOptionsAndTags(int cid, uint32_t owner, CFArrayRef spaces, uint32_t options, uint64_t *set_tags, uint64_t *clear_tags);
extern CFTypeRef SLSWindowQueryWindows(int cid, CFArrayRef windows, uint32_t options);
extern CFTypeRef SLSWindowQueryResultCopyWindows(CFTypeRef window_query);
extern int SLSWindowIteratorGetCount(CFTypeRef iterator);
extern bool SLSWindowIteratorAdvance(CFTypeRef iterator);
extern uint32_t SLSWindowIteratorGetParentID(CFTypeRef iterator);
extern uint32_t SLSWindowIteratorGetWindowID(CFTypeRef iterator);
extern uint64_t SLSWindowIteratorGetTags(CFTypeRef iterator);
extern uint64_t SLSWindowIteratorGetAttributes(CFTypeRef iterator);
extern CGError SLSRegisterNotifyProc(void* callback, uint32_t event, void* context);
extern CGError SLSRequestNotificationsForWindows(int cid, uint32_t* wid_list, uint32_t list_count);

struct app_window {
  uint32_t wid;
  uint64_t sid;
  pid_t pid;
};

struct app_windows {
  struct app_window* windows;
  uint32_t num_windows;
};

void begin_receiving_space_window_events();
void forced_space_windows_event();
