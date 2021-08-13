#ifndef GRAPH_DATA_H
#define GRAPH_DATA_H

struct graph_data {
  struct rgba_color color;
  bool fill;
  uint32_t cursor;
  uint32_t graph_width;
  float* x;
  float* y;
};

void graph_data_init(struct graph_data* graph_data, uint32_t graph_width);
void graph_data_push_back(struct graph_data* graph_data, float x, float y);
void graph_data_destruct(struct graph_data* graph_data);
float graph_data_get_x(struct graph_data* graph_data, uint32_t i);
float graph_data_get_y(struct graph_data* graph_data, uint32_t i);


#endif


