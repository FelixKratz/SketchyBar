#ifndef IMAGE_H
#define IMAGE_H

#include <_types/_uint16_t.h>
#include <_types/_uint32_t.h>
struct image {
  CGImageRef image_ref;
  CFDataRef data_ref;
  CGRect bounds;
};

void image_init(struct image* image);
bool image_set(struct image* image, CGImageRef new_image_ref, CGRect bounds, bool forced);
void image_load(struct image* image, char* path);
void image_calculate_bounds(struct image* image, uint32_t x, uint32_t y);
void image_draw(struct image* image, CGContextRef context);
void image_destroy(struct image* image);

#endif
