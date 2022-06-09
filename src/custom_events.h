#pragma once
#include "misc/helpers.h"

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
