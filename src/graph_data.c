#include "graph_data.h"
void graph_data_init(struct graph_data* graph_data, uint32_t graph_width) {
  graph_data->graph_width = graph_width;
  graph_data->y = malloc(sizeof(float) * graph_width);
  memset(graph_data->y, 0, sizeof(float) * graph_width);
  graph_data->cursor = 0;
  graph_data->color = rgba_color_from_hex(0xcccccc);
  graph_data->fill = true;
  graph_data->ready = true;
}

void graph_data_destruct(struct graph_data* graph_data) {
  if (!graph_data->ready) return;
  free(graph_data->y);
}

float graph_data_get_y(struct graph_data* graph_data, uint32_t i) {
  if (!graph_data->ready) return 0.f;
  return graph_data->y[ (graph_data->cursor + i)%graph_data->graph_width ];
}

void graph_data_push_back(struct graph_data* graph_data, float y) {
  if (!graph_data->ready) return;
  graph_data->y[graph_data->cursor] = y;

  ++graph_data->cursor;
  graph_data->cursor %= graph_data->graph_width;
}
