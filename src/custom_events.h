#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

extern void* g_workspace_context;

struct custom_event {
  char* name;
  char* notification;
};

struct custom_events {
  // How many events are reserved for system events
  uint32_t flag_offset;
  uint32_t count;
  
  struct custom_event** events;
};

void custom_event_init(struct custom_event* custom_event, char* name, char* notification);
void custom_events_init(struct custom_events* custom_events);
void custom_events_append(struct custom_events* custom_events, char* name, char* notification);
uint32_t custom_events_get_flag_for_name(struct custom_events* custom_events, char* name);
char* custom_events_get_name_for_notification(struct custom_events* custom_events, char* notification);
#endif
