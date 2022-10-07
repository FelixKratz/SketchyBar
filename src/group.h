#pragma once
#include "bar_item.h"

struct bar;

struct group {
  CGRect bounds;
  uint32_t num_members;
  struct bar_item** members;
};

struct group* group_create();
void group_init(struct group* group);
void group_set_name(struct group* group, char* _name);
void group_add_member(struct group* group, struct bar_item* item);
void group_remove_member(struct group* group, struct bar_item* bar_item);
uint32_t group_get_length(struct group* group, struct bar* bar);
uint32_t group_get_height(struct group* group, struct bar* bar);
bool group_is_first_member(struct group* group, struct bar_item* item);
struct bar_item* group_get_first_member(struct group* group);

void group_calculate_bounds(struct group* group, struct bar* bar, uint32_t x, uint32_t y, bool rtl);
void group_draw(struct group* group, CGContextRef context);
void group_destroy(struct group* group);

void group_serialize(struct group* group, char* indent, FILE* rsp);
