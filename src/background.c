#include "background.h"
#include "image.h"
#include "misc/helpers.h"
#include "shadow.h"

void background_init(struct background* background) {
  background->enabled = false;
  background->overrides_height = false;

  background->bounds.size.height = 25;
  background->bounds.size.width = 0;
  background->border_width = 0;
  background->padding_left = 0;
  background->padding_right = 0;
  background->corner_radius = 0;

  background->color = rgba_color_from_hex(0x00000000);
  background->border_color = rgba_color_from_hex(0x00000000);
  shadow_init(&background->shadow);
  image_init(&background->image);
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
  if (background->bounds.size.height == height) return false;
  background->bounds.size.height = height;
  background->overrides_height = height != 0;
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

void background_calculate_bounds(struct background* background, uint32_t x, uint32_t y) {
  background->bounds.origin.x = x;
  background->bounds.origin.y = y - background->bounds.size.height / 2;

  if (background->image.enabled)
    image_calculate_bounds(&background->image, x, y);
}

void background_draw(struct background* background, CGContextRef context) {
  if (!background->enabled) return;
  if (background->shadow.enabled) {
    CGRect bounds = shadow_get_bounds(&background->shadow, background->bounds);
    draw_rect(context, bounds, &background->shadow.color, background->corner_radius, background->border_width, &background->shadow.color, false);
  }

  if (background->image.enabled)
    image_draw(&background->image, context);

  draw_rect(context, background->bounds, &background->color, background->corner_radius, background->border_width, &background->border_color, false);
}

void background_destroy(struct background* background) {
  image_destroy(&background->image);
}

static bool background_parse_sub_domain(struct background* background, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_DRAWING))
    return background_set_enabled(background, evaluate_boolean_state(get_token(&message), background->enabled));
  else if (token_equals(property, PROPERTY_HEIGHT))
    return background_set_height(background, token_to_uint32t(get_token(&message)));
  else if (token_equals(property, PROPERTY_CORNER_RADIUS))
    return background_set_corner_radius(background, token_to_uint32t(get_token(&message)));
  else if (token_equals(property, PROPERTY_BORDER_WIDTH))
    return background_set_border_width(background, token_to_uint32t(get_token(&message)));
  else if (token_equals(property, PROPERTY_COLOR))
    return background_set_color(background, token_to_uint32t(get_token(&message)));
  else if (token_equals(property, PROPERTY_BORDER_COLOR))
    return background_set_border_color(background, token_to_uint32t(get_token(&message)));
  else if (token_equals(property, PROPERTY_PADDING_LEFT))
    return background_set_padding_left(background, token_to_int(get_token(&message)));
  else if (token_equals(property, PROPERTY_PADDING_RIGHT))
    return background_set_padding_right(background, token_to_int(get_token(&message)));
  else if (token_equals(property, SUB_DOMAIN_IMAGE))
    return image_load(&background->image, token_to_string(get_token(&message)), rsp);
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text, '.');
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
      struct token entry = { key_value_pair.value, strlen(key_value_pair.value) };
      if (token_equals(subdom, SUB_DOMAIN_SHADOW))
        return shadow_parse_sub_domain(&background->shadow, rsp, entry, message);
      else if (token_equals(subdom, SUB_DOMAIN_IMAGE))
        return image_parse_sub_domain(&background->image, rsp, entry, message);
      else {
        fprintf(rsp, "Invalid subdomain: %s \n", subdom.text);
        printf("Invalid subdomain: %s \n", subdom.text);
      }
    }
    else {
      fprintf(rsp, "Unknown property: %s \n", property.text);
      printf("Unknown property: %s \n", property.text);
    }
  }
  return false;
}
