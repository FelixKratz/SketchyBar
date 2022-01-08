#include "image.h"
#include "misc/helpers.h"
#include <_types/_uint32_t.h>
#include <string.h>

void image_init(struct image* image) {
  image->enabled = false;
  image->image_ref = NULL;
  image->data_ref = NULL;
  image->bounds = CGRectNull;
  image->size = CGSizeZero;
  image->scale = 1.0;
}

bool image_set_enabled(struct image* image, bool enabled) {
  if (image->enabled == enabled) return false;
  image->enabled = enabled;
  return true;
}

bool image_load(struct image* image, char* path, FILE* rsp) {
  char* res_path = resolve_path(path);
  if (!file_exists(res_path)) {
    printf("File %s not found!\n", res_path);
    fprintf(rsp, "File %s not found!\n", res_path);
    free(res_path);
    return false;
  }

  CGDataProviderRef data_provider = CGDataProviderCreateWithFilename(res_path);
  CGImageRef new_image_ref = NULL;
  if (strlen(res_path) > 3 && string_equals(&res_path[strlen(res_path) - 4], ".png"))
    new_image_ref = CGImageCreateWithPNGDataProvider(data_provider, NULL, false, kCGRenderingIntentDefault);
  else {
    new_image_ref = CGImageCreateWithJPEGDataProvider(data_provider, NULL, false, kCGRenderingIntentDefault);
  }
  if (data_provider && new_image_ref)
    image_set_image(image, new_image_ref, (CGRect){{0,0},{CGImageGetHeight(new_image_ref),CGImageGetWidth(new_image_ref)}}, true);
  else printf("Could not open image file at: %s!\n", res_path), fprintf(rsp, "Could not open image file at: %s!\n", res_path);

  CGDataProviderRelease(data_provider);
  free(res_path);
  return true;
}

bool image_data_equals(struct image* image, CFDataRef new_data_ref) {
  bool equals = false;
  if (image->image_ref && image->data_ref) {
    uint32_t old_len = CFDataGetLength(image->data_ref);
    uint32_t new_len = CFDataGetLength(new_data_ref);
    if (old_len == new_len)
      equals = memcmp(CFDataGetBytePtr(image->data_ref), CFDataGetBytePtr(new_data_ref), old_len) == 0;
  }

  return equals;
}

void image_copy(struct image* image, CGImageRef source) {
  if (source) image->image_ref = CGImageCreateCopy(source);
}

bool image_set_image(struct image* image, CGImageRef new_image_ref, CGRect bounds, bool forced) {
  CFDataRef new_data_ref = CGDataProviderCopyData(CGImageGetDataProvider(new_image_ref));

  if (!forced && image_data_equals(image, new_data_ref)) {
    CFRelease(new_data_ref);
    CGImageRelease(new_image_ref);
    return false;
  }

  if (image->image_ref) CGImageRelease(image->image_ref);
  if (image->data_ref) CFRelease(image->data_ref);

  image->size = bounds.size;
  image->bounds = (CGRect){{bounds.origin.x, bounds.origin.y},{bounds.size.width * image->scale, bounds.size.height * image->scale}};
  image->image_ref = new_image_ref;
  image->data_ref = new_data_ref;
  image->enabled = true;
  return true;
}

bool image_set_scale(struct image* image, float scale) {
  if (scale == image->scale) return false;
  image->scale = scale;
  image->bounds = (CGRect){{image->bounds.origin.x, image->bounds.origin.y},{image->size.width * image->scale, image->size.height * image->scale}};
  return true;
}

void image_calculate_bounds(struct image* image, uint32_t x, uint32_t y) {
  image->bounds.origin.x = x;
  image->bounds.origin.y = y - image->bounds.size.height / 2;
}

void image_draw(struct image* image, CGContextRef context) {
  if (!image->image_ref) return;
  CGContextDrawImage(context, image->bounds, image->image_ref);
}

void image_clear_pointers(struct image* image) {
  image->image_ref = NULL;
  image->data_ref = NULL;
}

void image_destroy(struct image* image) {
  CGImageRelease(image->image_ref);
  if (image->data_ref) CFRelease(image->data_ref);
  image_clear_pointers(image);
}

static bool image_parse_sub_domain(struct image* image, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_DRAWING))
    return image_set_enabled(image, evaluate_boolean_state(get_token(&message), image->enabled));
  else if (token_equals(property, PROPERTY_SCALE))
    return image_set_scale(image, token_to_float(get_token(&message)));
  else {
    fprintf(rsp, "Unknown property: %s \n", property.text);
    printf("Unknown property: %s \n", property.text);
  }
  return false;
}

