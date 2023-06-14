#pragma once
#include "misc/helpers.h"
#include "color.h"

struct graph {
  bool rtl;
  bool fill;
  bool enabled;
  bool overrides_fill_color;

  float* y;
  uint32_t width;
  uint32_t cursor;
  float line_width;

  CGRect bounds;
  struct color line_color;
  struct color fill_color;
};

void graph_init(struct graph* graph);
void graph_setup(struct graph* graph, uint32_t width);
void graph_push_back(struct graph* graph, float y);
float graph_get_y(struct graph* graph, uint32_t i);
uint32_t graph_get_length(struct graph* graph);

void graph_calculate_bounds(struct graph* graph, uint32_t x, uint32_t y, uint32_t height);
void graph_draw(struct graph* graph, CGContextRef context);
void graph_destroy(struct graph* graph);

void graph_serialize(struct graph* graph, char* indent, FILE* rsp);
bool graph_parse_sub_domain(struct graph* graph, FILE* rsp, struct token property, char* message);
