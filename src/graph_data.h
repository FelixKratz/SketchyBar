#ifndef GRAPH_DATA_H
#define GRAPH_DATA_H

struct graph_data {
  // Functional
  bool ready;
  uint32_t cursor;
  uint32_t graph_width;
  float* y;

  // Visual
  bool fill;
  bool overrides_fill_color;
  struct rgba_color line_color;
  struct rgba_color fill_color;
  float line_width;
};

void graph_data_init(struct graph_data* graph_data, uint32_t graph_width);
void graph_data_destroy(struct graph_data* graph_data);
void graph_data_push_back(struct graph_data* graph_data, float y);
void graph_data_destruct(struct graph_data* graph_data);
float graph_data_get_y(struct graph_data* graph_data, uint32_t i);


#endif


