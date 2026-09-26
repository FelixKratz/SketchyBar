#pragma once
#include "shadow.h"
#include "misc/defines.h"

extern CGImageRef workspace_icon_for_app(char* app);

struct image {
  bool enabled;

  float scale;
  CGSize size;
  CGRect bounds;
  
  char* path;

  CGImageRef image_ref;
  CFDataRef data_ref;

  struct shadow shadow;

  struct color border_color;
  float border_width;
  uint32_t corner_radius;

  int padding_left;
  int padding_right;
  int y_offset;

  struct image* link;

  bool is_symbol;
  char* symbol_name;
  float variable_value;
  int variable_value_mode;
  bool symbol_color_set;
  struct color symbol_color;
};

void image_init(struct image* image);
bool image_set_enabled(struct image* image, bool enabled);
void image_copy(struct image* image, CGImageRef source);
bool image_set_image(struct image* image, CGImageRef new_image_ref, CGRect bounds, bool forced);
bool image_load(struct image* image, char* path, FILE* rsp);
bool image_set_scale(struct image* image, float scale);
bool image_set_variable_value(struct image* image, float value);
bool image_set_variable_value_mode(struct image* image, int mode);
bool image_set_symbol_color(struct image* image, uint32_t color);

CGSize image_get_size(struct image* image);
void image_calculate_bounds(struct image* image, uint32_t x, uint32_t y);
void image_draw(struct image* image, CGContextRef context);
void image_clear_pointers(struct image* image);
void image_destroy(struct image* image);

void image_serialize(struct image* image, char* indent, FILE* rsp);
bool image_parse_sub_domain(struct image* image, FILE* rsp, struct token property, char* message);
