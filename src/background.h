#ifndef BACKGROUND_H
#define BACKGROUND_H

struct background {
  bool enabled;
  uint32_t height;
  uint32_t corner_radius;
  uint32_t border_width;
  int padding_left;
  int padding_right;
  struct rgba_color color;
  struct rgba_color border_color;
};

void background_init(struct background* background);
bool background_set_color(struct background* background, uint32_t color);
bool background_set_border_color(struct background* background, uint32_t color);
bool background_set_enabled(struct background* background, bool enabled);
bool background_set_height(struct background* background, uint32_t height);
bool background_set_border_width(struct background* background, uint32_t border_width);
bool background_set_corner_radius(struct background* background, uint32_t corner_radius);
bool background_set_padding_left(struct background* background, uint32_t pad);
bool background_set_padding_right(struct background* background, uint32_t pad);

static bool background_parse_sub_domain(struct background* background, FILE* rsp, struct token property, char* message);

#endif // !BACKGROUND_H
