#include "color.h"
#include "bar_manager.h"
#include "animation.h"

static bool color_update_hex(struct color* color) {
  uint32_t prev = color->hex;
  color->hex = (((uint32_t)(color->a * 255.f)) << 24)
               + (((uint32_t)(color->r * 255.f)) << 16)
               + (((uint32_t)(color->g * 255.f)) << 8)
               + (((uint32_t)(color->b * 255.f)) << 0);

  return prev != color->hex;
}

void color_init(struct color* color, uint32_t hex) {
  color_set_hex(color, hex);
}

bool color_set_hex(struct color* color, uint32_t hex) {
  color->a = clamp(((hex >> 24) & 0xff) / 255.f, 0.f, 1.f);
  color->r = clamp(((hex >> 16) & 0xff) / 255.f, 0.f, 1.f);
  color->g = clamp(((hex >> 8) & 0xff) / 255.f, 0.f, 1.f);
  color->b = clamp(((hex >> 0) & 0xff) / 255.f, 0.f, 1.f);
  return color_update_hex(color);
}

bool color_set_alpha(struct color* color, float alpha) {
  color->a = clamp(alpha, 0.f, 1.f);
  return color_update_hex(color);
}

bool color_set_r(struct color* color, float red) {
  color->r = clamp(red, 0.f, 1.f);
  return color_update_hex(color);
}

bool color_set_g(struct color* color, float green) {
  color->g = clamp(green, 0.f, 1.f);
  return color_update_hex(color);
}

bool color_set_b(struct color* color, float blue) {
  color->b = clamp(blue, 0.f, 1.f);
  return color_update_hex(color);
}

bool color_parse_sub_domain(struct color* color, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;

  if (token_equals(property, PROPERTY_COLOR_HEX)) {
    ANIMATE_BYTES(color_set_hex,
                  color,
                  color->hex,
                  token_to_int(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_COLOR_ALPHA)) {
    ANIMATE_FLOAT(color_set_alpha,
                  color,
                  color->a,
                  token_to_float(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_COLOR_RED)) {
    ANIMATE_FLOAT(color_set_r,
                  color,
                  color->r,
                  token_to_float(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_COLOR_GREEN)) {
    ANIMATE_FLOAT(color_set_g,
                  color,
                  color->g,
                  token_to_float(get_token(&message)));
  }
  else if (token_equals(property, PROPERTY_COLOR_BLUE)) {
    ANIMATE_FLOAT(color_set_b,
                  color,
                  color->b,
                  token_to_float(get_token(&message)));
  } else {
    respond(rsp, "[?] Color: Invalid property '%s'\n", property);
  }

  return needs_refresh;
}
