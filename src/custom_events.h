#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

#define UPDATE_FRONT_APP_SWITCHED 1
#define UPDATE_SPACE_CHANGE       1 << 1
#define UPDATE_DISPLAY_CHANGE     1 << 2
#define UPDATE_SYSTEM_WOKE        1 << 3
#define UPDATE_MOUSE_ENTERED      1 << 4
#define UPDATE_MOUSE_EXITED       1 << 5
#define UPDATE_MOUSE_CLICKED      1 << 6

extern void* g_workspace_context;

struct custom_event {
  char* name;
  char* notification;
};

struct custom_events {
  uint32_t count;
  
  struct custom_event** events;
};

void custom_event_init(struct custom_event* custom_event, char* name, char* notification);
void custom_events_init(struct custom_events* custom_events);
void custom_events_append(struct custom_events* custom_events, char* name, char* notification);
uint32_t custom_events_get_flag_for_name(struct custom_events* custom_events, char* name);
char* custom_events_get_name_for_notification(struct custom_events* custom_events, char* notification);
#endif
