#pragma once
#include "misc/helpers.h"
#include "color.h"

struct shadow {
  bool enabled;

  uint32_t angle;
  uint32_t distance;
  CGPoint offset;   

  struct color color;
};

void shadow_init(struct shadow* shadow);
CGRect shadow_get_bounds(struct shadow* shadow, CGRect reference_bounds);

void shadow_serialize(struct shadow* shadow, char* indent, FILE* rsp);
bool shadow_parse_sub_domain(struct shadow* shadow, FILE* rsp, struct token property, char* message);
