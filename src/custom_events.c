#include "custom_events.h"

static struct custom_event* custom_event_create(void) {
  return malloc(sizeof(struct custom_event));
}

void custom_event_init(struct custom_event* custom_event, char* name, char* notification) {
  custom_event->name = name;
  custom_event->notification = notification;
}

void custom_event_destroy(struct custom_event* custom_event) {
  if (custom_event->name) free(custom_event->name);
  if (custom_event->notification) free(custom_event->notification);
  free(custom_event);
}

void custom_events_init(struct custom_events* custom_events) {
  custom_events->count = 0;
  custom_events->events = NULL;

  // System Events
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_FRONT_APP_SWITCHED), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_SPACE_CHANGE), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_DISPLAY_CHANGE), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_SYSTEM_WOKE), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_MOUSE_ENTERED), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_MOUSE_EXITED), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_MOUSE_CLICKED), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_MOUSE_SCROLLED), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_SYSTEM_WILL_SLEEP), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_MOUSE_ENTERED_GLOBAL), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_MOUSE_EXITED_GLOBAL), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_MOUSE_SCROLLED_GLOBAL), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_VOLUME_CHANGE), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_BRIGHTNESS_CHANGE), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_POWER_SOURCE_CHANGE), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_WIFI_CHANGE), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_MEDIA_CHANGE), NULL);
  custom_events_append(custom_events, string_copy(COMMAND_SUBSCRIBE_SPACE_WINDOWS_CHANGE), NULL);
}

void custom_events_append(struct custom_events* custom_events, char* name, char* notification) {
  if (custom_events_get_flag_for_name(custom_events, name) > 0) { 
    if (name) free(name);
    if (notification) free(notification);
    return; 
  }
  custom_events->count++;
  custom_events->events = (struct custom_event**) realloc(
                          custom_events->events,
                          sizeof(struct custom_event*) * custom_events->count);

  custom_events->events[custom_events->count - 1] = custom_event_create();
  custom_events->events[custom_events->count - 1]->name = name;
  custom_events->events[custom_events->count - 1]->notification = notification;
  if (notification)
    workspace_create_custom_observer(&g_workspace_context, notification);
}

uint64_t custom_events_get_flag_for_name(struct custom_events* custom_events, char* name) {
  for (int i = 0; i < custom_events->count; i++) {
    if (strcmp(name, custom_events->events[i]->name) == 0) {
      return 1ULL << i;
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

void custom_events_destroy(struct custom_events* custom_events) {
  for (int i = 0; i < custom_events->count; i++) {
    custom_event_destroy(custom_events->events[i]);
  }
  free(custom_events->events);
}

void custom_events_serialize(struct custom_events* custom_events, FILE* rsp) {
  fprintf(rsp, "{\n");
  for (int i = 0; i < custom_events->count; i++) {
    fprintf(rsp, "\t\"%s\": {\n"
                 "\t\t\"bit\": %llu,\n"
                 "\t\t\"notification\": \"%s\"\n",
                 custom_events->events[i]->name,
                 1ULL << i,
                 custom_events->events[i]->notification);
    if (i < custom_events->count - 1) fprintf(rsp, "\t},\n");
  }
  fprintf(rsp, "\t}\n}\n");
}
