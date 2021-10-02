#include "graph.h"
void graph_init(struct graph* graph, uint32_t width) {
  graph->width = width;
  graph->y = malloc(sizeof(float) * width);
  memset(graph->y, 0, sizeof(float) * width);
  graph->cursor = 0;

  graph->line_color = rgba_color_from_hex(0xcccccc);
  graph->fill_color = rgba_color_from_hex(0xcccccc);
  graph->line_width = 0.5; 
  graph->fill = true;
  graph->overrides_fill_color = false;
  graph->enabled = true;
}

void graph_destroy(struct graph* graph) {
  if (!graph->enabled) return;
  free(graph->y);
}

float graph_get_y(struct graph* graph, uint32_t i) {
  if (!graph->enabled) return 0.f;
  return graph->y[ (graph->cursor + i)%graph->width ];
}

void graph_push_back(struct graph* graph, float y) {
  if (!graph->enabled) return;
  graph->y[graph->cursor] = y;

  ++graph->cursor;
  graph->cursor %= graph->width;
}

uint32_t graph_get_length(struct graph* graph) {
  if (graph->enabled) return graph->width;
  return 0;
}
