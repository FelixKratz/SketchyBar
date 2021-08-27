struct custom_event* custom_event_create(void) {
  return malloc(sizeof(struct custom_event));
}

void custom_event_init(struct custom_event* custom_event, char* name, char* notification) {
  custom_event->name = name;
  custom_event->notification = notification;
}

void custom_events_init(struct custom_events* custom_events) {
  custom_events->flag_offset = 4;
  custom_events->count = 0;
}


void custom_events_append(struct custom_events* custom_events, char* name, char* notification) {
  custom_events->count++;
  custom_events->events = (struct custom_event**) realloc(custom_events->events, sizeof(struct custom_event*) * custom_events->count);
  custom_events->events[custom_events->count - 1] = custom_event_create();
  custom_events->events[custom_events->count - 1]->name = name;
  custom_events->events[custom_events->count - 1]->notification = notification;
  if (notification)
    workspace_create_custom_observer(&g_workspace_context, notification);
} 

uint32_t custom_events_get_flag_for_name(struct custom_events* custom_events, char* name) {
  for (int i = 0; i < custom_events->count; i++) {
    if (strcmp(name, custom_events->events[i]->name) == 0) {
      return 1 << (i + custom_events->flag_offset);
    }
  }
  return 0;
}

char* custom_events_get_name_for_notification(struct custom_events* custom_events, char* notification) {
  for (int i = 0; i < custom_events->count; i++) {
    if (!custom_events->events[i]->notification) continue;
    if (strcmp(notification, custom_events->events[i]->notification) == 0) {
      return custom_events->events[i]->name;
    }
  }
  return NULL;
}

