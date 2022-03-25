#include "shadow.h"

void shadow_init(struct shadow* shadow) {
  shadow->enabled = false;
  shadow->angle = 30;
  shadow->distance = 5;
  shadow->color = rgba_color_from_hex(0xff000000);
}

bool shadow_set_enabled(struct shadow* shadow, bool enabled) {
  if (shadow->enabled == enabled) return false;
  shadow->enabled = enabled;
  return true;
}

bool shadow_set_angle(struct shadow* shadow, float angle) {
  if (shadow->angle == angle) return false;
  shadow->angle = angle;
  return true;
}

bool shadow_set_distance(struct shadow* shadow, float distance) {
  if (shadow->distance == distance) return false;
  shadow->distance = distance;
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
  return (CGRect){{reference_bounds.origin.x
                    + shadow->distance*cos(((double)shadow->angle)/360.
                    * 2.* M_PI                                         ),
                   reference_bounds.origin.y
                    - shadow->distance* sin(((double)shadow->angle)/360.
                    * 2.* M_PI                                          )},
                   reference_bounds.size                                   };
}

bool shadow_parse_sub_domain(struct shadow* shadow, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_DRAWING))
    return shadow_set_enabled(shadow,
                              evaluate_boolean_state(get_token(&message),
                              shadow->enabled                            ));

  else if (token_equals(property, PROPERTY_DISTANCE))
    return shadow_set_distance(shadow, token_to_uint32t(get_token(&message)));
  else if (token_equals(property, PROPERTY_ANGLE))
    return shadow_set_angle(shadow, token_to_uint32t(get_token(&message)));
  else if (token_equals(property, PROPERTY_COLOR))
    return shadow_set_color(shadow, token_to_uint32t(get_token(&message)));
  else {
    fprintf(rsp, "Unknown property: %s \n", property.text);
    printf("Unknown property: %s \n", property.text);
  }
  return false;
}

