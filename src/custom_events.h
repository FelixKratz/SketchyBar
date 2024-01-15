#pragma once
#include "misc/helpers.h"

#define UPDATE_FRONT_APP_SWITCHED   1ULL
#define UPDATE_SPACE_CHANGE         (1ULL << 1)
#define UPDATE_DISPLAY_CHANGE       (1ULL << 2)
#define UPDATE_SYSTEM_WOKE          (1ULL << 3)
#define UPDATE_MOUSE_ENTERED        (1ULL << 4)
#define UPDATE_MOUSE_EXITED         (1ULL << 5)
#define UPDATE_MOUSE_CLICKED        (1ULL << 6)
#define UPDATE_MOUSE_SCROLLED       (1ULL << 7)
#define UPDATE_SYSTEM_WILL_SLEEP    (1ULL << 8)
#define UPDATE_ENTERED_GLOBAL       (1ULL << 9)
#define UPDATE_EXITED_GLOBAL        (1ULL << 10)
#define UPDATE_SCROLLED_GLOBAL      (1ULL << 11)
#define UPDATE_VOLUME_CHANGE        (1ULL << 12)
#define UPDATE_BRIGHTNESS_CHANGE    (1ULL << 13)
#define UPDATE_POWER_SOURCE_CHANGE  (1ULL << 14)
#define UPDATE_WIFI_CHANGE          (1ULL << 15)
#define UPDATE_MEDIA_CHANGE         (1ULL << 16)
#define UPDATE_SPACE_WINDOWS_CHANGE (1ULL << 17)

extern void* g_workspace_context;
extern void workspace_create_custom_observer(void** context, char* name);

struct custom_event {
  char* name;
  char* notification;
};

void custom_event_init(struct custom_event* custom_event, char* name, char* notification);

struct custom_events {
  uint32_t count;
  struct custom_event** events;
};

void custom_events_init(struct custom_events* custom_events);
void custom_events_append(struct custom_events* custom_events, char* name, char* notification);
uint64_t custom_events_get_flag_for_name(struct custom_events* custom_events, char* name);
char* custom_events_get_name_for_notification(struct custom_events* custom_events, char* notification);
void custom_events_destroy(struct custom_events* custom_events);

void custom_events_serialize(struct custom_events* custom_events, FILE* rsp);
