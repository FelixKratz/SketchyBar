#include "group.h"
#include "background.h"
#include "bar_item.h"
#include <malloc/_malloc.h>
#include <stdint.h>
#include <string.h>

struct group* group_create() {
  struct group* group = malloc(sizeof(struct group));
  memset(group, 0, sizeof(struct group));
  return group;
}

void group_init(struct group* group) {
  group->num_members = 0;
  group->members = NULL;
}

bool group_is_item_member(struct group* group, struct bar_item* item) {
  for (uint32_t i = 0; i < group->num_members; i++) {
    if (group->members[i] == item) return true;
  }
  return false;
}

void group_add_member(struct group* group, struct bar_item* item) {
  if (group_is_item_member(group, item)) return;
  group->num_members++;
  group->members = realloc(group->members, sizeof(struct bar_item*)*group->num_members);
  group->members[group->num_members - 1] = item;
  item->group = group;
}

bool group_is_first_member(struct group* group, struct bar_item* item) {
  if (!group_is_item_member(group, item)) return false;
  if (group->num_members > 1) {return group->members[1] == item; }
  return false;
}

uint32_t group_get_length(struct group* group) {
  uint32_t length = 0;
  for (int i = 1; i < group->num_members; i++) {
    if (!group->members[i]->nospace || (group->members[i]->nospace && i == group->num_members - 1))
      if (group->members[i]->drawing)
        length += bar_item_get_length(group->members[i]);
  }
  return length;
}

void group_destroy(struct group* group) {
  for (int i = 0; i < group->num_members; i++) {
    group->members[i]->group = NULL;
  }
  if (group->members) free(group->members);
  free(group);
}
