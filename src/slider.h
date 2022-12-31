#pragma once

#include "background.h"
#include "text.h"

struct slider {
  uint32_t percentage;
  uint32_t foreground_color;

  CGRect bounds;
  struct text knob;
  struct background background;
  struct background foreground;
};

void slider_init(struct slider* slider);
void slider_setup(struct slider* slider, uint32_t width);
void slider_calculate_bounds(struct slider* slider, uint32_t x, uint32_t y);
void slider_draw(struct slider* slider, CGContextRef context);

uint32_t slider_get_length(struct slider* slider);

void slider_destroy(struct slider* slider);
void slider_serialize(struct slider* slider, char* indent, FILE* rsp);
bool slider_parse_sub_domain(struct slider* graph, FILE* rsp, struct token property, char* message);
