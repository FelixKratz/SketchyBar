#include "graph_data.h"
void graph_data_init(struct graph_data* graph_data, uint32_t graph_width) {
  graph_data->graph_width = graph_width;
  graph_data->x = malloc(sizeof(float) * graph_width);
  graph_data->y = malloc(sizeof(float) * graph_width);
  memset(graph_data->x, 0, sizeof(float) * graph_width);
  memset(graph_data->y, 0, sizeof(float) * graph_width);
  graph_data->cursor = 0;
  graph_data->color = rgba_color_from_hex(0xcccccc);
  graph_data->fill = true;
}

void graph_data_destruct(struct graph_data* graph_data) {
  free(graph_data->x);
  free(graph_data->y);
}

float graph_data_get_x(struct graph_data* graph_data, uint32_t i) {
  return graph_data->x[ (graph_data->cursor + i)%graph_data->graph_width ];
}

float graph_data_get_y(struct graph_data* graph_data, uint32_t i) {
  return graph_data->y[ (graph_data->cursor + i)%graph_data->graph_width ];
}

void graph_data_push_back(struct graph_data* graph_data, float x, float y) {
  graph_data->x[graph_data->cursor] = x;
  graph_data->y[graph_data->cursor] = y;

  ++graph_data->cursor;
  graph_data->cursor %= graph_data->graph_width;
}
