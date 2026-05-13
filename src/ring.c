#include "ring.h"
#include "bar_manager.h"
#include "animation.h"

#include <math.h>

#define RING_CAP_BUTT   'b'
#define RING_CAP_ROUND  'r'
#define RING_CAP_SQUARE 's'

static const float ring_full_circle = 2.f * M_PI;

static float ring_clamp_value(float value) {
  if (!isfinite(value)) return 0.f;
  if (value < 0.f) return 0.f;
  if (value > 1.f) return 1.f;
  return value;
}

static const char* ring_bool_string(bool value) {
  return value ? "true" : "false";
}

static const char* ring_cap_string(char cap) {
  switch (cap) {
    case RING_CAP_BUTT: return "butt";
    case RING_CAP_SQUARE: return "square";
    case RING_CAP_ROUND:
    default: return "round";
  }
}

static CGLineCap ring_line_cap(char cap) {
  switch (cap) {
    case RING_CAP_BUTT: return kCGLineCapButt;
    case RING_CAP_SQUARE: return kCGLineCapSquare;
    case RING_CAP_ROUND:
    default: return kCGLineCapRound;
  }
}

static char ring_cap_from_token(struct token token) {
  if (token_equals(token, "round")) return RING_CAP_ROUND;
  if (token_equals(token, "butt")) return RING_CAP_BUTT;
  if (token_equals(token, "square")) return RING_CAP_SQUARE;
  return 0;
}

static bool ring_set_value(struct ring* ring, float value) {
  value = ring_clamp_value(value);
  if (ring->value == value) return false;
  ring->value = value;
  return true;
}

static bool ring_set_width(struct ring* ring, int width) {
  if (width <= 0) return false;

  bool changed = false;
  uint32_t target_width = (uint32_t)width;
  if (ring->width != target_width) {
    ring->width = target_width;
    changed = true;
  }

  if (ring->line_width > (float)ring->width) {
    ring->line_width = (float)ring->width;
    changed = true;
  }

  return changed;
}

static bool ring_set_line_width(struct ring* ring, float line_width) {
  if (line_width <= 0.f) return false;

  float target_line_width = min(line_width, (float)ring->width);
  if (ring->line_width == target_line_width) return false;
  ring->line_width = target_line_width;
  return true;
}

static bool ring_set_start_angle(struct ring* ring, float start_angle) {
  if (ring->start_angle == start_angle) return false;
  ring->start_angle = start_angle;
  return true;
}

static bool ring_set_clockwise(struct ring* ring, bool clockwise) {
  if (ring->clockwise == clockwise) return false;
  ring->clockwise = clockwise;
  return true;
}

static bool ring_set_cap(struct ring* ring, char cap) {
  if (ring->cap == cap) return false;
  ring->cap = cap;
  return true;
}

static bool ring_set_color(struct ring* ring, uint32_t color) {
  return color_set_hex(&ring->color, color);
}

static bool ring_set_track_color(struct ring* ring, uint32_t color) {
  return color_set_hex(&ring->track_color, color);
}

void ring_init(struct ring* ring) {
  ring->enabled = true;
  ring->clockwise = true;
  ring->width = 20;
  ring->value = 0.f;
  ring->line_width = 2.f;
  ring->start_angle = 270.f;
  ring->cap = RING_CAP_ROUND;
  ring->bounds = (CGRect){{0, 0}, {0, 0}};

  color_init(&ring->color, 0xffffffff);
  color_init(&ring->track_color, 0x33ffffff);
}

void ring_setup(struct ring* ring, uint32_t width) {
  ring_set_width(ring, (int)width);
  ring->enabled = true;
}

uint32_t ring_get_length(struct ring* ring) {
  return ring->enabled ? ring->width : 0;
}

uint32_t ring_get_height(struct ring* ring) {
  return ring->enabled ? ring->width : 0;
}

void ring_calculate_bounds(struct ring* ring, uint32_t x, uint32_t y) {
  ring->bounds.origin.x = x;
  ring->bounds.origin.y = y - (float)ring->width / 2.f;
  ring->bounds.size.width = ring->width;
  ring->bounds.size.height = ring->width;
}

void ring_draw(struct ring* ring, CGContextRef context) {
  if (!ring->enabled || ring->width == 0 || ring->line_width <= 0.f) return;
  if (ring->track_color.a == 0.f && (ring->color.a == 0.f || ring->value <= 0.f)) return;

  float line_width = min(ring->line_width, (float)ring->width);
  if (line_width <= 0.f) return;

  float radius = ((float)ring->width - line_width) / 2.f;
  if (radius < 0.f) return;

  CGPoint center = CGPointMake(ring->bounds.origin.x + ring->bounds.size.width / 2.f,
                               ring->bounds.origin.y + ring->bounds.size.height / 2.f);

  float start = -ring->start_angle * deg_to_rad;
  bool cg_clockwise = ring->clockwise;

  CGContextSaveGState(context);
  CGContextSetLineWidth(context, line_width);
  CGContextSetLineCap(context, ring_line_cap(ring->cap));

  if (ring->track_color.a > 0.f) {
    float end = start + (ring->clockwise ? -ring_full_circle : ring_full_circle);
    CGContextSetRGBStrokeColor(context,
                               ring->track_color.r,
                               ring->track_color.g,
                               ring->track_color.b,
                               ring->track_color.a);
    CGContextAddArc(context, center.x, center.y, radius, start, end, cg_clockwise);
    CGContextStrokePath(context);
  }

  if (ring->color.a > 0.f && ring->value > 0.f) {
    float delta = ring_clamp_value(ring->value) * ring_full_circle;
    float end = start + (ring->clockwise ? -delta : delta);
    CGContextSetRGBStrokeColor(context,
                               ring->color.r,
                               ring->color.g,
                               ring->color.b,
                               ring->color.a);
    CGContextAddArc(context, center.x, center.y, radius, start, end, cg_clockwise);
    CGContextStrokePath(context);
  }

  CGContextRestoreGState(context);
}

void ring_serialize(struct ring* ring, char* indent, FILE* rsp) {
  fprintf(rsp, "%s\"value\": \"%f\",\n"
               "%s\"percentage\": \"%f\",\n"
               "%s\"color\": \"0x%x\",\n"
               "%s\"track_color\": \"0x%x\",\n"
               "%s\"line_width\": \"%f\",\n"
               "%s\"width\": \"%u\",\n"
               "%s\"start_angle\": \"%f\",\n"
               "%s\"clockwise\": \"%s\",\n"
               "%s\"cap\": \"%s\"",
               indent, ring->value,
               indent, ring->value * 100.f,
               indent, ring->color.hex,
               indent, ring->track_color.hex,
               indent, ring->line_width,
               indent, ring->width,
               indent, ring->start_angle,
               indent, ring_bool_string(ring->clockwise),
               indent, ring_cap_string(ring->cap));
}

void ring_destroy(struct ring* ring) {
  (void)ring;
}

bool ring_parse_sub_domain(struct ring* ring, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;

  if (token_equals(property, PROPERTY_VALUE)) {
    float value = ring_clamp_value(token_to_float(get_token(&message)));
    ANIMATE_FLOAT(ring_set_value,
                  ring,
                  ring->value,
                  value);
  }
  else if (token_equals(property, PROPERTY_PERCENTAGE)) {
    float value = ring_clamp_value(token_to_float(get_token(&message)) / 100.f);
    ANIMATE_FLOAT(ring_set_value,
                  ring,
                  ring->value,
                  value);
  }
  else if (token_equals(property, PROPERTY_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(ring_set_color,
                  ring,
                  ring->color.hex,
                  token_to_uint32t(token));
  }
  else if (token_equals(property, PROPERTY_TRACK_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(ring_set_track_color,
                  ring,
                  ring->track_color.hex,
                  token_to_uint32t(token));
  }
  else if (token_equals(property, PROPERTY_LINE_WIDTH)) {
    struct token token = get_token(&message);
    float line_width = token_to_float(token);
    if (line_width <= 0.f) {
      respond(rsp, "[!] Ring: Invalid line_width '%s'\n", token.text);
    } else {
      ANIMATE_FLOAT(ring_set_line_width,
                    ring,
                    ring->line_width,
                    line_width);
    }
  }
  else if (token_equals(property, PROPERTY_WIDTH)) {
    struct token token = get_token(&message);
    int width = token_to_int(token);
    if (width <= 0) {
      respond(rsp, "[!] Ring: Invalid width '%s'\n", token.text);
    } else {
      ANIMATE(ring_set_width,
              ring,
              ring->width,
              width);
    }
  }
  else if (token_equals(property, PROPERTY_START_ANGLE)) {
    struct token token = get_token(&message);
    ANIMATE_FLOAT(ring_set_start_angle,
                  ring,
                  ring->start_angle,
                  token_to_float(token));
  }
  else if (token_equals(property, PROPERTY_CLOCKWISE)) {
    needs_refresh = ring_set_clockwise(ring,
                                       evaluate_boolean_state(get_token(&message),
                                                              ring->clockwise));
  }
  else if (token_equals(property, PROPERTY_CAP)) {
    struct token token = get_token(&message);
    char cap = ring_cap_from_token(token);
    if (cap == 0) {
      respond(rsp, "[!] Ring: Invalid cap '%s'\n", token.text);
    } else {
      needs_refresh = ring_set_cap(ring, cap);
    }
  }
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text, '.');
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
      struct token entry = { key_value_pair.value, strlen(key_value_pair.value) };
      if (token_equals(subdom, SUB_DOMAIN_COLOR)) {
        return color_parse_sub_domain(&ring->color, rsp, entry, message);
      }
      else if (token_equals(subdom, PROPERTY_TRACK_COLOR)) {
        return color_parse_sub_domain(&ring->track_color, rsp, entry, message);
      }
      else {
        respond(rsp, "[!] Ring: Invalid subdomain '%s'\n", subdom.text);
      }
    }
    else {
      respond(rsp, "[!] Ring: Invalid property '%s'\n", property.text);
    }
  }

  return needs_refresh;
}
