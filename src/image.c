#include "image.h"
#include "misc/helpers.h"
#include "shadow.h"
#include "workspace.h"
#include "media.h"

void image_init(struct image* image) {
  image->enabled = false;
  image->image_ref = NULL;
  image->data_ref = NULL;
  image->bounds = CGRectNull;
  image->size = CGSizeZero;
  image->scale = 1.0;
  image->path = NULL;
  image->corner_radius = 0;
  image->border_width = 0;
  image->y_offset = 0;
  image->padding_left = 0;
  image->padding_right = 0;
  image->link = NULL;

  shadow_init(&image->shadow);
  color_init(&image->border_color, 0xcccccccc);
}

bool image_set_enabled(struct image* image, bool enabled) {
  if (image->enabled == enabled) return false;
  image->enabled = enabled;
  return true;
}

bool image_set_link(struct image* image, struct image* link) {
  if (image->link == link) return false;
  image->link = link;
  if (link) image->enabled = true;
  return true;
}

bool image_load(struct image* image, char* path, FILE* rsp) {
  if (!path) return false;
  char* app = string_copy(path);
  if (image->path) free(image->path);
  image->path = string_copy(path);
  char* res_path = resolve_path(path);
  CGImageRef new_image_ref = NULL;
  float scale = 1.f;

  struct key_value_pair app_kv = get_key_value_pair(app, '.');
  if (app_kv.key && app_kv.value && strcmp(app_kv.key, "app") == 0) {
    CGImageRef app_icon = workspace_icon_for_app(app_kv.value);
    scale = workspace_get_scale();
    scale *= scale;
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
  } else if (strcmp(path, "media.artwork") == 0) {
    free(res_path);
    free(app);
    begin_receiving_media_events();
    return image_set_link(image, &g_bar_manager.current_artwork);
  } else if (file_exists(res_path)) {
    CGDataProviderRef data_provider = CGDataProviderCreateWithFilename(res_path);
    if (data_provider) {
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
      CFRelease(data_provider);
    } else {
      respond(rsp, "[!] Image: Invalid Image Format: '%s'\n", app_kv.value);
      free(res_path);
      free(app);
      return false;
    }
  }
  else if (strlen(res_path) == 0) {
    image_destroy(image);
    free(res_path);
    free(app);
    return false;
  } else {
    respond(rsp, "[!] Image: File '%s' not found\n", res_path);
    free(res_path);
    free(app);
    return false;
  }

  if (new_image_ref) {
    image_set_image(image,
                    new_image_ref,
                    (CGRect){{0,0},
                             {CGImageGetWidth(new_image_ref) / scale,
                              CGImageGetHeight(new_image_ref) / scale }},
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

static bool image_data_equals(struct image* image, CFDataRef new_data_ref) {
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
  if (!new_image_ref) {
    if (image->image_ref) CGImageRelease(image->image_ref);
    if (image->data_ref) CFRelease(image->data_ref);
    image->image_ref = NULL;
    image->data_ref = NULL;
    return false;
  }
  if (image->link) image_set_link(image, NULL);
  CFDataRef new_data_ref = CGDataProviderCopyData(CGImageGetDataProvider(new_image_ref));

  if (!forced && image_data_equals(image, new_data_ref)
      && CGSizeEqualToSize(image->size, bounds.size)   ) {
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

bool image_set_corner_radius(struct image* image, uint32_t corner_radius) {
  if (image->corner_radius == corner_radius) return false;

  image->corner_radius = corner_radius;
  return true;
}

bool image_set_border_width(struct image* image, float border_width) {
  if (image->border_width == border_width) return false;

  image->border_width = border_width;
  return true;
}

bool image_set_border_color(struct image* image, uint32_t color) {
  return color_set_hex(&image->border_color, color);
}

bool image_set_padding_left(struct image* image, int padding_left) {
  if (image->padding_left == padding_left) return false;

  image->padding_left = padding_left;
  return true;
}

bool image_set_padding_right(struct image* image, int padding_right) {
  if (image->padding_right == padding_right) return false;

  image->padding_right = padding_right;
  return true;
}

bool image_set_yoffset(struct image* image, int yoffset) {
  if (image->y_offset == yoffset) return false;

  image->y_offset = yoffset;
  return true;
}

CGSize image_get_size(struct image* image) {
  return (CGSize){ .width = image->bounds.size.width
                            + image->padding_left
                            + image->padding_right
                            + (image->shadow.enabled
                               ? image->shadow.offset.x : 0),
                   .height = image->bounds.size.height
                             + 2*abs(image->y_offset) };
}

void image_calculate_bounds(struct image* image, uint32_t x, uint32_t y) {
  if (image->link && image->link->image_ref) {
    float internal_scale = 32.f / CGImageGetHeight(image->link->image_ref);
    CGRect bounds = (CGRect){{0,0},
                  { CGImageGetWidth(image->link->image_ref) * internal_scale,
                    32.f                                                    }};

    image->size = bounds.size;
    image->bounds = (CGRect){{0, 0},
                             {bounds.size.width * image->scale,
                              bounds.size.height * image->scale}};
  }

  image->bounds.origin.x = x + image->padding_left;
  image->bounds.origin.y = y - image->bounds.size.height / 2 + image->y_offset;
}

void image_draw(struct image* image, CGContextRef context) {
  if ((!image->link && !image->image_ref)
      || (image->link && !image->link->image_ref)) return;

  if (image->shadow.enabled) {
    CGContextSaveGState(context);
    CGRect sbounds = shadow_get_bounds(&image->shadow, image->bounds);
    CGContextSetRGBFillColor(context, image->shadow.color.r, image->shadow.color.g, image->shadow.color.b, image->shadow.color.a);
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathAddRoundedRect(path,
                         NULL,
                         sbounds,
                         image->corner_radius,
                         image->corner_radius);

    CGContextAddPath(context, path);
    CGContextDrawPath(context, kCGPathFillStroke);
    CFRelease(path);
    CGContextRestoreGState(context);
  } 

  CGContextSaveGState(context);
  if (image->bounds.size.height > 2*image->corner_radius
      && image->bounds.size.width > 2*image->corner_radius) {
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathAddRoundedRect(path,
                         NULL,
                         image->bounds,
                         image->corner_radius,
                         image->corner_radius);

    CGContextAddPath(context, path);
    CGContextClip(context);
    CFRelease(path);
  }

  CGContextDrawImage(context,
                     image->bounds,
                     image->link ? image->link->image_ref : image->image_ref);

  if (image->bounds.size.height > 2*image->corner_radius
      && image->bounds.size.width > 2*image->corner_radius) {
    CGContextSetLineWidth(context, 2*image->border_width);
    CGContextSetRGBStrokeColor(context,
                               image->border_color.r,
                               image->border_color.g,
                               image->border_color.b,
                               image->border_color.a);

    CGContextSetRGBFillColor(context, 0, 0, 0, 0);
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathAddRoundedRect(path,
                         NULL,
                         image->bounds,
                         image->corner_radius,
                         image->corner_radius);

    CGContextAddPath(context, path);
    CGContextDrawPath(context, kCGPathFillStroke);
    CFRelease(path);
  }
  CGContextRestoreGState(context);
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
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_STRING)) {
    return image_load(image, token_to_string(get_token(&message)), rsp);
  }
  else if (token_equals(property, PROPERTY_DRAWING)) {
    return image_set_enabled(image,
                             evaluate_boolean_state(get_token(&message),
                             image->enabled)                            );
  }
  else if (token_equals(property, PROPERTY_SCALE)) {
    ANIMATE_FLOAT(image_set_scale,
                  image,
                  image->scale,
                  token_to_float(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_CORNER_RADIUS)) {
    ANIMATE(image_set_corner_radius,
            image,
            image->corner_radius,
            token_to_uint32t(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_PADDING_LEFT)) {
    ANIMATE(image_set_padding_left,
            image,
            image->padding_left,
            token_to_int(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_PADDING_RIGHT)) {
    ANIMATE(image_set_padding_right,
            image,
            image->padding_right,
            token_to_int(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_YOFFSET)) {
    ANIMATE(image_set_yoffset,
            image,
            image->y_offset,
            token_to_int(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_BORDER_WIDTH)) {
    ANIMATE_FLOAT(image_set_border_width,
                  image,
                  image->border_width,
                  token_to_float(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_BORDER_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(image_set_border_color,
                  image,
                  image->border_color.hex,
                  token_to_int(token));
  }
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text,
                                                              '.'           );
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = {key_value_pair.key,strlen(key_value_pair.key)};
      struct token entry = {key_value_pair.value,strlen(key_value_pair.value)};
      if (token_equals(subdom, SUB_DOMAIN_BORDER_COLOR)) {
        return color_parse_sub_domain(&image->border_color,
                                      rsp,
                                      entry,
                                      message);
      }
      else if (token_equals(subdom, SUB_DOMAIN_SHADOW)) {
        return shadow_parse_sub_domain(&image->shadow,
                                       rsp,
                                       entry,
                                       message             );
      }
      else {
        respond(rsp, "[?] Image: Invalid subdomain: %s \n", property.text);
      }
    }
    else {
        respond(rsp, "[?] Image: Unknown property: %s \n", property.text);
    }
  }
  return needs_refresh;
}
