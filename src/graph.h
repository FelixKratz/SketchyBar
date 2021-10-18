#ifndef GRAPH_H
#define GRAPH_H

#include <_types/_uint32_t.h>
struct graph {
  // Functional
  bool enabled;
  uint32_t cursor;
  uint32_t width;
  float* y;

  // Visual
  bool fill;
  bool overrides_fill_color;
  struct rgba_color line_color;
  struct rgba_color fill_color;
  float line_width;
};

void graph_init(struct graph* graph, uint32_t width);
void graph_destroy(struct graph* graph);
void graph_push_back(struct graph* graph, float y);
void graph_destruct(struct graph* graph);
float graph_get_y(struct graph* graph, uint32_t i);
uint32_t graph_get_length(struct graph* graph);

static bool graph_parse_sub_domain(struct graph* graph, FILE* rsp, struct token property, char* message);

#endif


