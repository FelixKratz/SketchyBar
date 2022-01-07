#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

#define UPDATE_FRONT_APP_SWITCHED 1ULL
#define UPDATE_SPACE_CHANGE       1ULL << 1
#define UPDATE_DISPLAY_CHANGE     1ULL << 2
#define UPDATE_SYSTEM_WOKE        1ULL << 3
#define UPDATE_MOUSE_ENTERED      1ULL << 4
#define UPDATE_MOUSE_EXITED       1ULL << 5
#define UPDATE_MOUSE_CLICKED      1ULL << 6
#define UPDATE_SYSTEM_WILL_SLEEP  1ULL << 7

extern void* g_workspace_context;

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

#endif
