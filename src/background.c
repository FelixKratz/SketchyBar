#include "background.h"
#include "misc/helpers.h"

void background_init(struct background* background) {
  background->enabled = false;

  background->height = 0;
  background->border_width = 0;
  background->padding_left = 0;
  background->padding_right = 0;
  background->corner_radius = 0;

  background->color = rgba_color_from_hex(0xff000000);
  background->border_color = rgba_color_from_hex(0xff000000);
}

bool background_set_color(struct background* background, uint32_t color) {
  struct rgba_color target_color = rgba_color_from_hex(color);
  if (background->color.r == target_color.r 
      && background->color.g == target_color.g 
      && background->color.b == target_color.b 
      && background->color.a == target_color.a) return false;
  background->color = target_color;
  background_set_enabled(background, true);
  return true;
}

bool background_set_border_color(struct background* background, uint32_t color) {
  struct rgba_color target_color = rgba_color_from_hex(color);
  if (background->border_color.r == target_color.r 
      && background->border_color.g == target_color.g 
      && background->border_color.b == target_color.b 
      && background->border_color.a == target_color.a) return false;
  background->border_color = target_color;
  return true;
}

bool background_set_enabled(struct background* background, bool enabled) {
  if (background->enabled == enabled) return false;
  background->enabled = enabled;
  return true;
}

bool background_set_height(struct background* background, uint32_t height) {
  if (background->height == height) return false;
  background->height = height;
  return true;
}

bool background_set_border_width(struct background* background, uint32_t border_width) {
  if (background->border_width == border_width) return false;
  background->border_width = border_width;
  return true;
}

bool background_set_corner_radius(struct background* background, uint32_t corner_radius) {
  if (background->corner_radius == corner_radius) return false;
  background->corner_radius = corner_radius;
  return true;
}

bool background_set_padding_left(struct background* background, uint32_t pad) {
  if (background->padding_left == pad) return false;
  background->padding_left = pad;
  return true;
}

bool background_set_padding_right(struct background* background, uint32_t pad) {
  if (background->padding_right == pad) return false;
  background->padding_right = pad;
  return true;
}
