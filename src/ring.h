#pragma once

#include <CoreGraphics/CoreGraphics.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "text.h"

struct ring {
  bool enabled;
  bool clockwise;
  uint32_t width;
  float value;
  float line_width;
  float start_angle;
  char cap;
  char marker_position;
  CGRect bounds;
  struct color color;
  struct color track_color;
  struct text marker;
};

void ring_init(struct ring* ring);
void ring_clear_pointers(struct ring* ring);
void ring_copy(struct ring* ring, struct ring* source);
void ring_setup(struct ring* ring, uint32_t width);
uint32_t ring_get_length(struct ring* ring);
uint32_t ring_get_height(struct ring* ring);
void ring_calculate_bounds(struct ring* ring, uint32_t x, uint32_t y);
void ring_draw(struct ring* ring, CGContextRef context);
void ring_serialize(struct ring* ring, char* indent, FILE* rsp);
void ring_destroy(struct ring* ring);
bool ring_parse_sub_domain(struct ring* ring, FILE* rsp, struct token property, char* message);
