#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

struct custom_events {
  // How many events are reserved for system events
  uint32_t flag_offset;
  uint32_t count;
  
  char** names;
};

void custom_event_append(char* name);
#endif
