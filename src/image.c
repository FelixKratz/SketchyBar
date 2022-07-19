#include "image.h"
#include "misc/helpers.h"
#include "shadow.h"

void image_init(struct image* image) {
  image->enabled = false;
  image->image_ref = NULL;
  image->data_ref = NULL;
  image->bounds = CGRectNull;
  image->size = CGSizeZero;
  image->scale = 1.0;
  image->path = NULL;

  shadow_init(&image->shadow);
}

bool image_set_enabled(struct image* image, bool enabled) {
  if (image->enabled == enabled) return false;
  image->enabled = enabled;
  return true;
}

bool image_load(struct image* image, char* path, FILE* rsp) {
  char* app = string_copy(path);
  image->path = string_copy(path);
  char* res_path = resolve_path(path);
  CGImageRef new_image_ref = NULL;

  struct key_value_pair app_kv = get_key_value_pair(app, '.');
  if (app_kv.key && app_kv.value && strcmp(app_kv.key, "app") == 0) {
    CGImageRef app_icon = workspace_icon_for_app(app_kv.value);
    if (app_icon) new_image_ref = app_icon;
    else {
      respond(rsp, "[!] Image: Invalid application name: '%s'\n", app_kv.value);
      free(res_path);
      free(app);
      return false;
    }
  } else if (app_kv.key && app_kv.value && strcmp(app_kv.key, "space") == 0) {
    uint32_t sid = atoi(app_kv.value);
    CGImageRef space_img = space_capture(sid);
    if (space_img) new_image_ref = space_img;
    else {
      respond(rsp, "[!] Image: Invalid Space ID: '%s'\n", app_kv.value);
      free(res_path);
      free(app);
      return false;
    }
  } else if (file_exists(res_path)) {
    CGDataProviderRef data_provider = CGDataProviderCreateWithFilename(res_path);
    if (strlen(res_path) > 3 && string_equals(&res_path[strlen(res_path) - 4], ".png"))
      new_image_ref = CGImageCreateWithPNGDataProvider(data_provider,
                                                       NULL,
                                                       false,
                                                       kCGRenderingIntentDefault);
    else {
      new_image_ref = CGImageCreateWithJPEGDataProvider(data_provider,
                                                        NULL,
                                                        false,
                                                        kCGRenderingIntentDefault);
    }

    if (data_provider) CFRelease(data_provider);
  }
  else {
    respond(rsp, "[!] Image: File '%s' not found\n", res_path);
    free(res_path);
    free(app);
    return false;
  }

  if (new_image_ref) {
    image_set_image(image,
                    new_image_ref,
                    (CGRect){{0,0},
                             {CGImageGetWidth(new_image_ref),
                              CGImageGetHeight(new_image_ref)}},
                    true                                        );
  }
  else {
    if (new_image_ref) CFRelease(new_image_ref);
    printf("Could not open image file at: %s\n", res_path);
    fprintf(rsp, "Could not open image file at: %s\n", res_path);
  }

  free(res_path);
  free(app);
  return true;
}

bool image_data_equals(struct image* image, CFDataRef new_data_ref) {
  bool equals = false;
  if (image->image_ref && image->data_ref) {
    uint32_t old_len = CFDataGetLength(image->data_ref);
    uint32_t new_len = CFDataGetLength(new_data_ref);
    if (old_len == new_len)
      equals = memcmp(CFDataGetBytePtr(image->data_ref),
                      CFDataGetBytePtr(new_data_ref),
                      old_len                           )
               == 0;
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
  image->bounds = (CGRect){{0, 0},
                           {bounds.size.width * image->scale,
                            bounds.size.height * image->scale}};

  image->image_ref = new_image_ref;
  image->data_ref = new_data_ref;
  image->enabled = true;
  return true;
}

bool image_set_scale(struct image* image, float scale) {
  if (scale == image->scale) return false;
  image->scale = scale;
  image->bounds = (CGRect){{image->bounds.origin.x, image->bounds.origin.y},
                           {image->size.width * image->scale,
                            image->size.height * image->scale}};
  return true;
}

void image_calculate_bounds(struct image* image, uint32_t x, uint32_t y) {
  image->bounds.origin.x = x;
  image->bounds.origin.y = y - image->bounds.size.height / 2;
}

void image_draw(struct image* image, CGContextRef context) {
  if (!image->image_ref) return;

  // if (image->shadow.enabled) {
  //   CGContextSaveGState(context);
  //   CGRect sbounds = shadow_get_bounds(&image->shadow, image->bounds);
  //   CGContextDrawImage(context, sbounds, image->image_ref);
  //   CGContextClipToMask(context, sbounds, image->image_ref);
  //   CGContextSetRGBFillColor(context, image->shadow.color.r, image->shadow.color.g, image->shadow.color.b, image->shadow.color.a);
  //   CGContextFillRect(context, sbounds);
  //   CGContextRestoreGState(context);
  // } 

  CGContextDrawImage(context, image->bounds, image->image_ref);
}

void image_clear_pointers(struct image* image) {
  image->image_ref = NULL;
  image->data_ref = NULL;
  image->path = NULL;
}

void image_destroy(struct image* image) {
  CGImageRelease(image->image_ref);
  if (image->data_ref) CFRelease(image->data_ref);
  if (image->path) free(image->path);
  image_clear_pointers(image);
}

void image_serialize(struct image* image, char* indent, FILE* rsp) {
  fprintf(rsp, "%s\"value\": \"%s\",\n"
               "%s\"drawing\": \"%s\",\n"
               "%s\"scale\": %f",
               indent, image->path,
               indent, format_bool(image->enabled),
               indent, image->scale                );
}

bool image_parse_sub_domain(struct image* image, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_DRAWING))
    return image_set_enabled(image,
                             evaluate_boolean_state(get_token(&message),
                             image->enabled)                            );
  else if (token_equals(property, PROPERTY_SCALE))
    return image_set_scale(image, token_to_float(get_token(&message)));
  else {
    fprintf(rsp, "Unknown property: %s \n", property.text);
    printf("Unknown property: %s \n", property.text);
  }
  return false;
}
