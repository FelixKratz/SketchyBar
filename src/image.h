#ifndef IMAGE_H
#define IMAGE_H

struct image {
  bool enabled;

  float scale;
  CGSize size;
  CGRect bounds;

  CGImageRef image_ref;
  CFDataRef data_ref;

  struct shadow shadow;
};

void image_init(struct image* image);
bool image_set_enabled(struct image* image, bool enabled);
bool image_data_equals(struct image* image, CFDataRef new_data_ref);
void image_copy(struct image* image, CGImageRef source);
bool image_set_image(struct image* image, CGImageRef new_image_ref, CGRect bounds, bool forced);
bool image_load(struct image* image, char* path, FILE* rsp);

void image_calculate_bounds(struct image* image, uint32_t x, uint32_t y);
void image_draw(struct image* image, CGContextRef context);
void image_clear_pointers(struct image* image);
void image_destroy(struct image* image);

static bool image_parse_sub_domain(struct image* image, FILE* rsp, struct token property, char* message);
#endif
