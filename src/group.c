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
  group->members = NULL;
}

bool group_is_item_member(struct group* group, struct bar_item* item) {
  for (uint32_t i = 0; i < group->num_members; i++) {
    if (group->members[i] == item) return true;
  }
  return false;
}

void group_add_item(struct group* group, struct bar_item* item) {
  if (group_is_item_member(group, item)) return;
  group->num_members++;
  group->members = realloc(group->members, sizeof(struct bar_item*)*group->num_members);
  group->members[group->num_members - 1] = item;
}

void group_destroy(struct group* group) {
  if (group->members) free(group->members);
  free(group);
}
