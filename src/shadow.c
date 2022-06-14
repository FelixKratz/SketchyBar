#include "shadow.h"
#include "bar_manager.h"

void shadow_init(struct shadow* shadow) {
  shadow->enabled = false;
  shadow->angle = 30;
  shadow->distance = 5;
  shadow->offset.x = ((float)shadow->distance)
                      *cos(((double)shadow->angle)*deg_to_rad);
  shadow->offset.y = -((float)shadow->distance)
                       *sin(((double)shadow->angle)*deg_to_rad);
  shadow->color = rgba_color_from_hex(0xff000000);
}

bool shadow_set_enabled(struct shadow* shadow, bool enabled) {
  if (shadow->enabled == enabled) return false;
  shadow->enabled = enabled;
  return true;
}

bool shadow_set_angle(struct shadow* shadow, uint32_t angle) {
  if (shadow->angle == angle) return false;
  shadow->angle = angle;
  shadow->offset.x = ((float)shadow->distance)*cos(((double)shadow->angle)*deg_to_rad);
  shadow->offset.y = -((float)shadow->distance)*sin(((double)shadow->angle)*deg_to_rad);
  return true;
}

bool shadow_set_distance(struct shadow* shadow, uint32_t distance) {
  if (shadow->distance == distance) return false;
  shadow->distance = distance;
  shadow->offset.x = ((float)shadow->distance)
                      *cos(((double)shadow->angle)*deg_to_rad);
  shadow->offset.y = -((float)shadow->distance)
                      *sin(((double)shadow->angle)*deg_to_rad);
  return true;
}

bool shadow_set_color(struct shadow* shadow, uint32_t color) {
  struct rgba_color target_color = rgba_color_from_hex(color);
  if (shadow->color.r == target_color.r 
      && shadow->color.g == target_color.g 
      && shadow->color.b == target_color.b 
      && shadow->color.a == target_color.a) return false;
  shadow->color = target_color;
  shadow_set_enabled(shadow, true);
  return true;
}

CGRect shadow_get_bounds(struct shadow* shadow, CGRect reference_bounds) {
  return (CGRect){{reference_bounds.origin.x + shadow->offset.x,
                   reference_bounds.origin.y + shadow->offset.y },
                   reference_bounds.size                          };
}

bool shadow_parse_sub_domain(struct shadow* shadow, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_DRAWING)) {
    needs_refresh = shadow_set_enabled(shadow,
                                       evaluate_boolean_state(get_token(&message),
                                                              shadow->enabled     ));
  }
  else if (token_equals(property, PROPERTY_DISTANCE)) {
    struct token token = get_token(&message);
    ANIMATE(shadow_set_distance,
            shadow,
            shadow->distance,
            token_to_int(token) );
  }
  else if (token_equals(property, PROPERTY_ANGLE)) {
    struct token token = get_token(&message);
    ANIMATE(shadow_set_angle,
            shadow,
            shadow->angle,
            token_to_int(token));
  }
  else if (token_equals(property, PROPERTY_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(shadow_set_color,
                  shadow,
                  hex_from_rgba_color(shadow->color),
                  token_to_int(token)                );
  }
  else {
    respond(rsp, "[!] Shadow: Invalid property '%s'\n", property.text);
  }

  return needs_refresh;
}

