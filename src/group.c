#include "group.h"
#include "background.h"
#include <malloc/_malloc.h>
#include <string.h>

struct group* group_create() {
  struct group* group = malloc(sizeof(struct group));
  memset(group, 0, sizeof(struct group));
  return group;
}

void group_init(struct group* group) {
  group->name = string_copy("");
  background_init(&group->background);
}

void group_set_name(struct group* group, char* _name) {
  if (group->name && group->name != _name) free(group->name);
  group->name = _name;
}

void group_destroy(struct group* group) {
  if (group->name) free(group->name);
  free(group);
}
