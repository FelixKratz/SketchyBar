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

  color_init(&shadow->color, 0xff000000);
}

static bool shadow_set_enabled(struct shadow* shadow, bool enabled) {
  if (shadow->enabled == enabled) return false;
  shadow->enabled = enabled;
  return true;
}

static bool shadow_set_angle(struct shadow* shadow, uint32_t angle) {
  if (shadow->angle == angle) return false;
  shadow->angle = angle;
  shadow->offset.x = ((float)shadow->distance)*cos(((double)shadow->angle)*deg_to_rad);
  shadow->offset.y = -((float)shadow->distance)*sin(((double)shadow->angle)*deg_to_rad);
  return true;
}

static bool shadow_set_distance(struct shadow* shadow, uint32_t distance) {
  if (shadow->distance == distance) return false;
  shadow->distance = distance;
  shadow->offset.x = ((float)shadow->distance)
                      *cos(((double)shadow->angle)*deg_to_rad);
  shadow->offset.y = -((float)shadow->distance)
                      *sin(((double)shadow->angle)*deg_to_rad);
  return true;
}

static bool shadow_set_color(struct shadow* shadow, uint32_t color) {
  bool changed = shadow_set_enabled(shadow, true);
  return color_set_hex(&shadow->color, color) || changed;
}

CGRect shadow_get_bounds(struct shadow* shadow, CGRect reference_bounds) {
  return (CGRect){{reference_bounds.origin.x + shadow->offset.x,
                   reference_bounds.origin.y + shadow->offset.y },
                   reference_bounds.size                          };
}

void shadow_serialize(struct shadow* shadow, char* indent, FILE* rsp) {
  fprintf(rsp, "%s\"drawing\": \"%s\",\n"
               "%s\"color\": \"0x%x\",\n"
               "%s\"angle\": %u,\n"
               "%s\"distance\": %u",
               indent, format_bool(shadow->enabled),
               indent, shadow->color.hex,
               indent, shadow->angle,
               indent, shadow->distance                   );
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
                  shadow->color.hex,
                  token_to_int(token)                );
  }
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text,
                                                              '.'           );
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = {key_value_pair.key,strlen(key_value_pair.key)};
      struct token entry = {key_value_pair.value,strlen(key_value_pair.value)};
      if (token_equals(subdom, SUB_DOMAIN_COLOR)) {
        return color_parse_sub_domain(&shadow->color, rsp, entry, message);
      }
      else {
        respond(rsp, "[!] Shadow: Invalid subdomain '%s'\n", subdom.text);
      }
    } else {
      respond(rsp, "[!] Shadow: Invalid property '%s'\n", property.text);
    }
  }

  return needs_refresh;
}

