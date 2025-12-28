#include "background.h"
#include "image.h"
#include "misc/helpers.h"
#include "shadow.h"
#include "animation.h"
#include "bar_manager.h"

void background_init(struct background* background) {
  background->enabled = false;
  background->clip = 0.f;
  background->clips = NULL;
  background->num_clips = 0;
  background->overrides_height = false;

  background->bounds.size.height = 0;
  background->bounds.size.width = 0;
  background->border_width = 0;
  background->padding_left = 0;
  background->padding_right = 0;
  background->corner_radius = 0;
  background->x_offset = 0;
  background->y_offset = 0;

  color_init(&background->color, 0x00000000);
  color_init(&background->border_color, 0x00000000);
  shadow_init(&background->shadow);
  image_init(&background->image);
}

bool background_set_height(struct background* background, uint32_t height) {
  if (background->bounds.size.height == height) return false;
  background->bounds.size.height = height;
  background->overrides_height = height != 0;
  return true;
}

static void background_reset_clip(struct background* background) {
  for (uint32_t i = 0; i < background->num_clips; i++)
    free(background->clips[i]);

  if (background->clips) free(background->clips);
  background->clips = NULL;
  background->num_clips = 0;
}

bool background_set_enabled(struct background* background, bool enabled) {
  if (background->enabled == enabled) return false;

  if (background_clips_bar(background)) {
    background_reset_clip(background);
    g_bar_manager.bar_needs_update = true;
  }

  background->enabled = enabled;

  return true;
}

bool background_set_color(struct background* background, uint32_t color) {
  bool changed = background_set_enabled(background, true);
  return color_set_hex(&background->color, color) || changed;
}

static bool background_set_clip(struct background* background, float clip) {
  if (background->clip == clip) return false;
  background->clip = clip;
  g_bar_manager.bar_needs_update = true;
  g_bar_manager.might_need_clipping = true;
  if (clip > 0.f) background_set_enabled(background, true);
  return true;
}

static bool background_set_border_color(struct background* background, uint32_t color) {
  return color_set_hex(&background->border_color, color);
}

static bool background_set_border_width(struct background* background, uint32_t border_width) {
  if (background->border_width == border_width) return false;
  background->border_width = border_width;
  return true;
}

static bool background_set_corner_radius(struct background* background, uint32_t corner_radius) {
  if (background->corner_radius == corner_radius) return false;
  background->corner_radius = corner_radius;
  return true;
}

static bool background_set_xoffset(struct background* background, int offset) {
  if (background->x_offset == offset) return false;
  background->x_offset = offset;
  return true;
}

static bool background_set_yoffset(struct background* background, int offset) {
  if (background->y_offset == offset) return false;
  background->y_offset = offset;
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

bool background_clip_needs_update(struct background* background, struct bar* bar) {
  if (background->clip == 0.f || !background->enabled) return false;
  struct background* clip = background_get_clip(background, bar->adid);
  if (!CGRectEqualToRect(background->bounds, clip->bounds)) return true;
  if (background->corner_radius != clip->corner_radius) return true;
  if (background->x_offset != clip->x_offset) return true;
  if (background->y_offset != clip->y_offset) return true;
  return false;
}

static void background_update_clip(struct background* background, struct background* clip) {
  memcpy(clip, background, sizeof(struct background));
  background_clear_pointers(clip);
}

struct background* background_get_clip(struct background* background, uint32_t adid) {
  if (adid < 1) return NULL;
  if (background->num_clips < adid) {
    background->clips = (struct background**) realloc(background->clips,
                                                sizeof(struct background*)*adid);
    memset(background->clips + background->num_clips,
           0,
           sizeof(struct background*) * (adid - background->num_clips));

    background->num_clips = adid;
  }

  if (!background->clips[adid - 1]) {
    struct background* clip = malloc(sizeof(struct background));
    background->clips[adid - 1] = clip;
    background_init(clip);
    background_update_clip(background, clip);
    clip->bounds.origin = g_nirvana;
  }

  return background->clips[adid - 1];
}

bool background_clips_bar(struct background* background) {
  return background->enabled && (background->clip > 0.f);
}

void background_clip_bar(struct background* background, int offset, struct bar* bar) {
  if (!background_clips_bar(background)) return;

  struct background* clip = background_get_clip(background, bar->adid);
  background_update_clip(background, clip);

  CGRect background_bounds = background->bounds;
  background_bounds.origin.x += offset + background->x_offset;
  background_bounds.origin.y += background->y_offset;

  clip_rect(bar->window.context,
            background_bounds,
            background->clip,
            background->corner_radius);
}

void background_calculate_bounds(struct background* background, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
  background->bounds.origin.x = x;
  background->bounds.origin.y = y - height / 2;
  background->bounds.size.width = width;
  background->bounds.size.height = height;

  if (background->image.enabled)
    image_calculate_bounds(&background->image, x, y);
}

static void draw_rect(CGContextRef context, CGRect region, struct color* fill_color, uint32_t corner_radius, uint32_t line_width, struct color* stroke_color) {
  CGContextSetLineWidth(context, line_width);
  if (stroke_color) CGContextSetRGBStrokeColor(context, stroke_color->r, stroke_color->g, stroke_color->b, stroke_color->a);
  CGContextSetRGBFillColor(context, fill_color->r, fill_color->g, fill_color->b, fill_color->a);

  CGMutablePathRef path = CGPathCreateMutable();
  CGRect inset_region = CGRectInset(region, (float)(line_width) / 2.f, (float)(line_width) / 2.f);
  if (corner_radius > inset_region.size.height / 2.f || corner_radius > inset_region.size.width / 2.f)
    corner_radius = inset_region.size.height > inset_region.size.width ? inset_region.size.width / 2.f : inset_region.size.height / 2.f; 
  CGPathAddRoundedRect(path, NULL, inset_region, corner_radius, corner_radius);
  CGContextAddPath(context, path);
  CGContextDrawPath(context, kCGPathFillStroke);
  CFRelease(path);
}

void background_draw(struct background* background, CGContextRef context) {
  if (!background->enabled) return;

  if ((background->border_color.a == 0 || background->border_width == 0)
      && (background->color.a == 0)
      && !background->shadow.enabled
      && !background->image.enabled                                      ) {
    // The background is enabled but has no content.
    return;
  }

  CGRect background_bounds = background->bounds;
  background_bounds.origin.x += background->x_offset;
  background_bounds.origin.y += background->y_offset;
  if (background->shadow.enabled) {
    CGRect bounds = shadow_get_bounds(&background->shadow, background_bounds);
    draw_rect(context,
              bounds,
              &background->shadow.color,
              background->corner_radius,
              background->border_width,
              &background->shadow.color);
  }

  draw_rect(context,
            background_bounds,
            &background->color,
            background->corner_radius,
            background->border_width,
            &background->border_color);

  if (background->image.enabled)
    image_draw(&background->image, context);

}

void background_clear_pointers(struct background* background) {
  background->clips = NULL;
  background->num_clips = 0;
  image_clear_pointers(&background->image);
}

void background_destroy(struct background* background) {
  for (uint32_t i = 0; i < background->num_clips; i++)
    free(background->clips[i]);

  if (background->clips) free(background->clips);

  image_destroy(&background->image);
  background_clear_pointers(background);
}

void background_serialize(struct background* background, char* indent, FILE* rsp, bool detailed) {
  fprintf(rsp, "%s\"drawing\": \"%s\",\n"
               "%s\"color\": \"0x%x\",\n"
               "%s\"border_color\": \"0x%x\",\n"
               "%s\"border_width\": %u,\n"
               "%s\"height\": %u,\n"
               "%s\"corner_radius\": %u,\n"
               "%s\"padding_left\": %d,\n"
               "%s\"padding_right\": %d,\n"
               "%s\"x_offset\": %d,\n"
               "%s\"y_offset\": %d,\n"
               "%s\"clip\": %f,\n",
               indent, format_bool(background->enabled),
               indent, background->color.hex,
               indent, background->border_color.hex,
               indent, background->border_width,
               indent, background->overrides_height ? (int)background->bounds.size.height : 0,
               indent, background->corner_radius,
               indent, background->padding_left,
               indent, background->padding_right,
               indent, background->x_offset,
               indent, background->y_offset,
               indent, background->clip                                                       );

  char deeper_indent[strlen(indent) + 2];
  snprintf(deeper_indent, strlen(indent) + 2, "%s\t", indent);

  fprintf(rsp, "%s\"image\": {\n", indent);
  image_serialize(&background->image, deeper_indent, rsp);
  fprintf(rsp, "\n%s}", indent);

  if (!detailed) return;

  fprintf(rsp, ",\n%s\"shadow\": {\n", indent);
  shadow_serialize(&background->shadow, deeper_indent, rsp);
  fprintf(rsp, "\n%s}", indent);
}

bool background_parse_sub_domain(struct background* background, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_DRAWING))
    return background_set_enabled(background,
                                  evaluate_boolean_state(get_token(&message),
                                                         background->enabled));
  else if (token_equals(property, PROPERTY_CLIP)) {
    struct token token = get_token(&message);
    ANIMATE_FLOAT(background_set_clip,
                  background,
                  background->clip,
                  token_to_float(token));
  } else if (token_equals(property, PROPERTY_HEIGHT)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_height,
            background,
            background->bounds.size.height,
            token_to_int(token)            );
  }
  else if (token_equals(property, PROPERTY_CORNER_RADIUS)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_corner_radius,
            background,
            background->corner_radius,
            token_to_int(token)          );
  }
  else if (token_equals(property, PROPERTY_BORDER_WIDTH)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_border_width,
            background,
            background->border_width,
            token_to_int(token)         );
  }
  else if (token_equals(property, PROPERTY_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(background_set_color,
                  background,
                  background->color.hex,
                  token_to_int(token)   );
  }
  else if (token_equals(property, PROPERTY_BORDER_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(background_set_border_color,
                  background,
                  background->border_color.hex,
                  token_to_int(token)          );
  }
  else if (token_equals(property, PROPERTY_PADDING_LEFT)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_padding_left,
            background,
            background->padding_left,
            token_to_int(token)         );
  }
  else if (token_equals(property, PROPERTY_PADDING_RIGHT)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_padding_right,
            background,
            background->padding_right,
            token_to_int(token)         );
  }
  else if (token_equals(property, PROPERTY_XOFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_xoffset,
            background,
            background->x_offset,
            token_to_int(token)    );
  }
  else if (token_equals(property, PROPERTY_YOFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(background_set_yoffset,
            background,
            background->y_offset,
            token_to_int(token)    );
  }
  else if (token_equals(property, SUB_DOMAIN_IMAGE)) {
    return image_load(&background->image,
                      token_to_string(get_token(&message)),
                      rsp                                  );
  }
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text,
                                                              '.'           );
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = {key_value_pair.key,strlen(key_value_pair.key)};
      struct token entry = {key_value_pair.value,strlen(key_value_pair.value)};
      if (token_equals(subdom, SUB_DOMAIN_SHADOW))
        return shadow_parse_sub_domain(&background->shadow,
                                       rsp,
                                       entry,
                                       message             );
      else if (token_equals(subdom, SUB_DOMAIN_IMAGE)) {
        return image_parse_sub_domain(&background->image, rsp, entry, message);
      }
      else if (token_equals(subdom, SUB_DOMAIN_COLOR)) {
        return color_parse_sub_domain(&background->color, rsp, entry, message);
      }
      else if (token_equals(subdom, SUB_DOMAIN_BORDER_COLOR)) {
        return color_parse_sub_domain(&background->border_color,
                                      rsp,
                                      entry,
                                      message                   );
      }
      else {
        respond(rsp, "[!] Background: Invalid subdomain '%s'\n", subdom.text);
      }
    }
    else {
      respond(rsp, "[!] Background: Invalid property '%s'\n", property.text);
    }
  }
  return needs_refresh;
}
