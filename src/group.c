#include "group.h"

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
  if (item->group && item->group->members && item->group->members[0] == item) {
    for (int i = 1; i < item->group->num_members; i++) {
      group_add_member(group, item->group->members[i]);
    }
  } else {
    group->num_members++;
    group->members = realloc(group->members, sizeof(struct bar_item*)*group->num_members);
    group->members[group->num_members - 1] = item;
    item->group = group;
  }
}

struct bar_item* group_get_first_member(struct group* group) {
  if (group->num_members > 1) return group->members[1];
  return NULL;
}

bool group_is_first_member(struct group* group, struct bar_item* item) {
  if (!group_is_item_member(group, item)) return false;
  if (group->num_members > 1) {return group->members[1] == item; }
  return false;
}

uint32_t group_get_length(struct group* group) {
  uint32_t length = 0;
  for (int i = 1; i < group->num_members; i++) {
    if (group->members[i]->drawing) {
      if (group->members[i]->position == POSITION_RIGHT) {
        if (i > 1 && !group->members[i]->has_const_width)
          length += group->members[i]->background.padding_right;

        if (i < group->num_members - 1 && !group->members[i]->has_const_width)
          length += group->members[i]->background.padding_left;
      }
      else {
        if (i > 1 && !group->members[i]->has_const_width)
          length += group->members[i]->background.padding_left;
        if (i < group->num_members - 1 && !group->members[i]->has_const_width)
          length += group->members[i]->background.padding_right;
      }

      length += bar_item_get_length(group->members[i], false);
    }
  }
  return length;
}

uint32_t group_count_members_drawn(struct group* group) {
  int count = 0;
  for (int i = 1; i < group->num_members; i++) {
    if (group->members[i]->drawing) {
      count++;
    }
  }
  return count;
}

void group_remove_member(struct group* group, struct bar_item* bar_item) {
  if (group->num_members <= 0) return;
  struct bar_item* tmp[group->num_members - 1];
  int count = 0;
  for (int i = 0; i < group->num_members; i++) {
    if (group->members[i] == bar_item) continue;
    tmp[count++] = group->members[i];
  }
  group->num_members--;
  group->members = realloc(group->members, sizeof(struct bar_item*)*group->num_members);
  memcpy(group->members, tmp, sizeof(struct bar_item*)*group->num_members);
}

void group_destroy(struct group* group) {
  for (int i = 0; i < group->num_members; i++) {
    group->members[i]->group = NULL;
  }
  if (group->members) free(group->members);
  free(group);
}

void group_calculate_bounds(struct group* group, uint32_t x, uint32_t y, bool rtl) {
  background_calculate_bounds(&group->members[0]->background, x, y);
  group->members[0]->background.bounds.size.width = group_get_length(group)
                                                    + (rtl
                                                       ? 0
                                                       : group_count_members_drawn(group));
  group->members[0]->background.bounds.origin.x = x;
  group->members[0]->background.bounds.origin.y = y - group->members[0]->background.bounds.size.height / 2
                                                  + group->members[0]->y_offset;
}

void group_draw(struct group* group, CGContextRef context) {
  background_draw(&group->members[0]->background, context);
}

void group_serialize(struct group* group, FILE* rsp) {
    fprintf(rsp, ",\n"
                 "\t\"bracket\": [\n");

    int counter = 0;
    for (int i = 1; i < group->num_members; i++) {
      if (!group->members[i]) continue;
      if (counter++ > 0) fprintf(rsp, ",\n");
      fprintf(rsp, "\t\t\"%s\"",
              group->members[i]->name);
    }
    fprintf(rsp, "\n\t]");
}

