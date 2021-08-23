void custom_events_init(struct custom_events* custom_events) {
  custom_events->flag_offset = 4;
  custom_events->count = 0;
}

void custom_event_append(struct custom_events* custom_events, char *name) {
  custom_events->count++;
  custom_events->names = (char**) realloc(custom_events->names, sizeof(char*) * custom_events->count);
  custom_events->names[custom_events->count - 1] = name;
} 

uint32_t custom_events_get_flag_for_name(struct custom_events* custom_events, char* name) {
  for (int i = 0; i < custom_events->count; i++) {
    if (strcmp(name, custom_events->names[i])) {
      return 1 << (i + custom_events->flag_offset);
    }
  }
  return 0;
}
