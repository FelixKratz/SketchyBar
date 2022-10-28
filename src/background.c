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

  background->bounds.size.height = 25;
  background->bounds.size.width = 0;
  background->border_width = 0;
  background->padding_left = 0;
  background->padding_right = 0;
  background->corner_radius = 0;
  background->y_offset = 0;

  background->color = rgba_color_from_hex(0x00000000);
  background->border_color = rgba_color_from_hex(0x00000000);
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

static bool background_set_enabled(struct background* background, bool enabled) {
  if (background->enabled == enabled) return false;

  if (background_clips_bar(background)) {
    background_reset_clip(background);
    g_bar_manager.bar_needs_update = true;
  }

  background->enabled = enabled;

  return true;
}

static bool background_set_clip(struct background* background, float clip) {
  if (background->clip == clip) return false;
  background->clip = clip;
  g_bar_manager.bar_needs_update = true;
  background_set_enabled(background, clip > 0.f);
  return true;
}

static bool background_set_color(struct background* background, uint32_t color) {
  struct rgba_color target_color = rgba_color_from_hex(color);
  if (background->color.r == target_color.r 
      && background->color.g == target_color.g 
      && background->color.b == target_color.b 
      && background->color.a == target_color.a) return false;
  background->color = target_color;
  background_set_enabled(background, true);
  return true;
}

static bool background_set_border_color(struct background* background, uint32_t color) {
  struct rgba_color target_color = rgba_color_from_hex(color);
  if (background->border_color.r == target_color.r 
      && background->border_color.g == target_color.g 
      && background->border_color.b == target_color.b 
      && background->border_color.a == target_color.a) return false;
  background->border_color = target_color;
  return true;
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

static bool background_set_padding_left(struct background* background, uint32_t pad) {
  if (background->padding_left == pad) return false;
  background->padding_left = pad;
  return true;
}

static bool background_set_padding_right(struct background* background, uint32_t pad) {
  if (background->padding_right == pad) return false;
  background->padding_right = pad;
  return true;
}

static bool background_set_yoffset(struct background* background, int offset) {
  if (background->y_offset == offset) return false;
  background->y_offset = offset;
  return true;
}

bool background_clip_needs_update(struct background* background, struct bar* bar) {
  if (background->clip == 0.f || !background->enabled) return false;
  struct background* clip = background_get_clip(background, bar->adid);
  if (!CGRectEqualToRect(background->bounds, clip->bounds)) return true;
  if (background->corner_radius != clip->corner_radius) return true;
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
  background_bounds.origin.x += offset;
  background_bounds.origin.y += background->y_offset;

  clip_rect(bar->window.context,
            background_bounds,
            background->clip,
            background->corner_radius);
}

void background_calculate_bounds(struct background* background, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
  background->bounds.origin.x = x;
  background->bounds.origin.y = y - background->bounds.size.height / 2;
  background->bounds.size.width = width;
  background->bounds.size.height = height;

  if (background->image.enabled)
    image_calculate_bounds(&background->image, x, y);
}

void background_draw(struct background* background, CGContextRef context) {
  if (!background->enabled) return;
  CGRect background_bounds = background->bounds;
  background_bounds.origin.y += background->y_offset;
  if (background->shadow.enabled) {
    CGRect bounds = shadow_get_bounds(&background->shadow, background_bounds);
    draw_rect(context,
              bounds,
              &background->shadow.color,
              background->corner_radius,
              background->border_width,
              &background->shadow.color,
              false                     );
  }

  if (background->image.enabled)
    image_draw(&background->image, context);

  draw_rect(context,
            background_bounds,
            &background->color,
            background->corner_radius,
            background->border_width,
            &background->border_color,
            false                     );
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
               "%s\"clip\": \"%f\",\n"
               "%s\"color\": \"0x%x\",\n"
               "%s\"border_color\": \"0x%x\",\n"
               "%s\"border_width\": %u,\n"
               "%s\"height\": %u,\n"
               "%s\"corner_radius\": %u,\n"
               "%s\"padding_left\": %d,\n"
               "%s\"padding_right\": %d,\n"
               "%s\"y_offset\": %d,\n",
               indent, format_bool(background->enabled),
               indent, background->clip,
               indent, hex_from_rgba_color(background->color),
               indent, hex_from_rgba_color(background->border_color),
               indent, background->border_width,
               indent, background->overrides_height ? (int)background->bounds.size.height : 0,
               indent, background->corner_radius,
               indent, background->padding_left,
               indent, background->padding_right,
               indent, background->y_offset                                                    );

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
                  hex_from_rgba_color(background->color),
                  token_to_int(token)                    );
  }
  else if (token_equals(property, PROPERTY_BORDER_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(background_set_border_color,
                  background,
                  hex_from_rgba_color(background->border_color),
                  token_to_int(token)                           );
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
      else if (token_equals(subdom, SUB_DOMAIN_IMAGE))
        return image_parse_sub_domain(&background->image, rsp, entry, message);
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
