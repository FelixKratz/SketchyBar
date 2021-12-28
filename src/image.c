#include "image.h"
#include "misc/helpers.h"
#include <_types/_uint32_t.h>

void image_init(struct image* image) {
  image->enabled = false;
  image->image_ref = NULL;
  image->data_ref = NULL;
  image->bounds = CGRectNull;
}

bool image_set_enabled(struct image* image, bool enabled) {
  if (image->enabled == enabled) return false;
  image->enabled = enabled;
  return true;
}

bool image_load(struct image* image, char* path) {
  CGDataProviderRef data_provider = CGDataProviderCreateWithFilename(path);
  CGImageRef new_image_ref = CGImageCreateWithPNGDataProvider(data_provider, NULL, false, kCGRenderingIntentDefault);
  if (data_provider && new_image_ref)
    image_set(image, new_image_ref, CGRectNull, true);
  else printf("Could find or open image file at: %s!\n", path);

  CGDataProviderRelease(data_provider);
  free(path);
  return true;
}

bool image_data_equals(struct image* image, CFDataRef new_data_ref) {
  bool equals = false;
  if (image->image_ref && image->data_ref) {
    uint32_t old_len = CFDataGetLength(image->data_ref);
    uint32_t new_len = CFDataGetLength(new_data_ref);
    if (old_len == new_len) {
      const unsigned char* old_data = CFDataGetBytePtr(image->data_ref);
      const unsigned char* new_data = CFDataGetBytePtr(new_data_ref);
      equals = true;
      for (int i = 0; i < old_len; i++) {
        if (old_data[i] != new_data[i]) {
          equals = false;
          break;
        }
      }
    }
  }

  return equals;
}

bool image_set(struct image* image, CGImageRef new_image_ref, CGRect bounds, bool forced) {
  CFDataRef new_data_ref = CGDataProviderCopyData(CGImageGetDataProvider(new_image_ref));

  if (!forced && image_data_equals(image, new_data_ref)) {
    CFRelease(new_data_ref);
    CGImageRelease(new_image_ref);
    return false;
  }

  if (image->image_ref) CGImageRelease(image->image_ref);
  if (image->data_ref) CFRelease(image->data_ref);

  image->bounds = bounds;
  image->image_ref = new_image_ref;
  image->data_ref = new_data_ref;
  image->enabled = true;
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

void image_destroy(struct image* image) {
  CGImageRelease(image->image_ref);
  if (image->data_ref) CFRelease(image->data_ref);
}

static bool image_parse_sub_domain(struct image* image, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_DRAWING))
    return image_set_enabled(image, evaluate_boolean_state(get_token(&message), image->enabled));
  else if (token_equals(property, "tmp"))
    return image_load(image, token_to_string(get_token(&message)));
  else {
    fprintf(rsp, "Unknown property: %s \n", property.text);
    printf("Unknown property: %s \n", property.text);
  }
  return false;
}

