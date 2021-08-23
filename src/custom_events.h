#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

struct custom_events {
  // How many events are reserved for system events
  uint32_t flag_offset;
  uint32_t count;
  
  char** names;
};

void custom_events_init(struct custom_events* custom_events);
void custom_events_append(struct custom_events* custom_events, char* name);
void custom_events_trigger(struct custom_events* custom_events, char* name);
uint32_t custom_events_get_flag_for_name(struct custom_events* custom_events, char* name);
#endif
