#pragma once
#include "misc/helpers.h"

struct shadow {
  bool enabled;

  uint32_t angle;
  uint32_t distance;

  struct rgba_color color;
};

void shadow_init(struct shadow* shadow);
bool shadow_set_enabled(struct shadow* shadow, bool enabled);
bool shadow_set_angle(struct shadow* shadow, uint32_t angle);
bool shadow_set_distance(struct shadow* shadow, uint32_t distance);
bool shadow_set_color(struct shadow* shadow, uint32_t color);
CGRect shadow_get_bounds(struct shadow* shadow, CGRect reference_bounds);

bool shadow_parse_sub_domain(struct shadow* shadow, FILE* rsp, struct token property, char* message);
