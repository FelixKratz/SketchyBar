#pragma once
#include "image.h"

struct background {
  bool enabled;
  float clip;
  bool overrides_height;

  int padding_left;
  int padding_right;
  int y_offset;
  uint32_t border_width;
  uint32_t corner_radius;

  CGRect bounds;
  struct image image;
  struct shadow shadow;
  struct color color;
  struct color border_color;

  struct background** clips;
  uint32_t num_clips;
};

struct bar;

void background_init(struct background* background);
void background_calculate_bounds(struct background* background, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

bool background_set_enabled(struct background* background, bool enabled);
bool background_set_color(struct background* background, uint32_t color);
bool background_set_height(struct background* background, uint32_t height);
bool background_set_padding_left(struct background* background, uint32_t pad);
bool background_set_padding_right(struct background* background, uint32_t pad);

void background_draw(struct background* background, CGContextRef context);

struct background* background_get_clip(struct background* background, uint32_t adid);
void background_clip_bar(struct background* background, int offset, struct bar* bar);
bool background_clip_needs_update(struct background* background, struct bar* bar);
bool background_clips_bar(struct background* background);

void background_clear_pointers(struct background* background);
void background_destroy(struct background* background);

void background_serialize(struct background* background, char* indent, FILE* rsp, bool detailed);
bool background_parse_sub_domain(struct background* background, FILE* rsp, struct token property, char* message);
