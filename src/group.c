#include "group.h"
#include "bar.h"

static struct bar_item* group_get_first_member(struct group* group, struct bar* bar) {
  if (group->num_members == 1) return NULL;

  int min = INT32_MAX;
  struct bar_item* first_item = NULL;

  for (int i = 1; i < group->num_members; i++) {
    struct bar_item* member = group->members[i];
    if (bar_draws_item(bar, member)) {
      struct window* window = bar_item_get_window(member, bar->adid);
      if (window->origin.x < min) {
        min = window->origin.x;
        first_item = member;
      }
    }
  }

  return first_item;
}

static struct bar_item* group_get_last_member(struct group* group, struct bar* bar) {
  if (group->num_members == 1) return NULL;

  int max = INT32_MIN;
  struct bar_item* last_item = NULL;

  for (int i = 1; i < group->num_members; i++) {
    struct bar_item* member = group->members[i];
    if (bar_draws_item(bar, member)) {
      struct window* window = bar_item_get_window(member, bar->adid);
      if (window->origin.x + window->frame.size.width > max) {
        max = window->origin.x + window->frame.size.width;
        last_item = member;
      }
    }
  }

  return last_item;
}

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
    group->members = realloc(group->members,
                             sizeof(struct bar_item*)*group->num_members);
    group->members[group->num_members - 1] = item;
    item->group = group;
  }
}

uint32_t group_get_length(struct group* group, struct bar* bar) {
  int len = group->last_window->origin.x
            + group->last_window->frame.size.width
            + group->last_item->background.padding_right
            + group->first_item->background.padding_left
            - group->first_window->origin.x;

  return max(len, 0);
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
  group->members = realloc(group->members,
                           sizeof(struct bar_item*)*group->num_members);
  memcpy(group->members, tmp, sizeof(struct bar_item*)*group->num_members);
}

void group_destroy(struct group* group) {
  for (int i = 0; i < group->num_members; i++) {
    group->members[i]->group = NULL;
  }
  if (group->members) free(group->members);
  free(group);
}

void group_calculate_bounds(struct group* group, struct bar* bar, uint32_t y) {
  group->first_item = group_get_first_member(group, bar);
  group->first_window = bar_item_get_window(group->first_item, bar->adid);

  group->last_item = group_get_last_member(group, bar);
  group->last_window = bar_item_get_window(group->last_item, bar->adid);

  if (!group->first_window || !group->last_window) {
    group->bounds.origin = g_nirvana;
    return;
  }

  uint32_t group_length = group_get_length(group, bar);
  CGPoint shadow_offsets = bar_item_calculate_shadow_offsets(group->members[0]);
  

  group->bounds = (CGRect){{group->first_window->origin.x
                            - group->first_item->background.padding_left,
                            group->first_window->origin.y},
                           {group_length
                            + shadow_offsets.x
                            + shadow_offsets.y,
                           group->first_window->frame.size.height}};
  
  background_calculate_bounds(&group->members[0]->background,
                              max(shadow_offsets.x, 0),
                              y + group->members[0]->y_offset,
                              group_get_length(group, bar),
                              group->members[0]->background.bounds.size.height);
}

void group_serialize(struct group* group, char* indent, FILE* rsp) {
    int counter = 0;
    for (int i = 1; i < group->num_members; i++) {
      if (!group->members[i]) continue;
      if (counter++ > 0) fprintf(rsp, ",\n");
      fprintf(rsp, "%s\"%s\"", indent, group->members[i]->name);
    }
}

