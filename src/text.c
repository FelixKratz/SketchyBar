#include "text.h"
#include "bar_manager.h"

static void text_calculate_truncated_width(struct text* text, CFDictionaryRef attributes) {
  if (text->max_chars > 0) {
    uint32_t len = strlen(text->string) + 4;
    char buffer[len];
    memset(buffer, 0, len);

    char* read = text->string;
    char* write = buffer;
    uint32_t counter = 0;
    while (*read) {
      if ((*read & 0xC0) != 0x80) counter++; 
      if (counter > text->max_chars) {
        break;
      }
      *write++ = *read++;
    }

    CFStringRef string = CFStringCreateWithCString(NULL,
                                                   buffer,
                                                   kCFStringEncodingUTF8);

    if (string) {
      CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL,
                                                                   string,
                                                                   attributes);

      CTLineRef line = CTLineCreateWithAttributedString(attr_string);

      CGRect bounds = CTLineGetBoundsWithOptions(line,
                                              kCTLineBoundsUseGlyphPathBounds);
      text->width = (uint32_t)(bounds.size.width + 1.5);
      CFRelease(attr_string);
      CFRelease(line);
      CFRelease(string);
    }
  }
}

static void text_prepare_line(struct text* text) {
  const void *keys[] = { kCTFontAttributeName,
                         kCTForegroundColorFromContextAttributeName };

  if (text->font.font_changed) {
    font_create_ctfont(&text->font);
    text->font.font_changed = false;
  }
  const void *values[] = { text->font.ct_font, kCFBooleanTrue };
  CFDictionaryRef attributes = CFDictionaryCreate(NULL,
                                                  keys,
                                                  values,
                                                  array_count(keys),
                                                  &kCFTypeDictionaryKeyCallBacks,
                                                  &kCFTypeDictionaryValueCallBacks);

  CFStringRef string = CFStringCreateWithCString(NULL,
                                                 text->string,
                                                 kCFStringEncodingUTF8);

  if (!string) string = CFStringCreateWithCString(NULL,
                                          "Warning: Malformed UTF-8 string",
                                          kCFStringEncodingUTF8             );

  CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL,
                                                               string,
                                                               attributes);

  text->line.line = CTLineCreateWithAttributedString(attr_string);

  CTLineGetTypographicBounds(text->line.line,
                             &text->line.ascent,
                             &text->line.descent,
                             NULL                );

  text->bounds = CTLineGetBoundsWithOptions(text->line.line,
                                            kCTLineBoundsUseGlyphPathBounds);

  text->bounds.size.width = (uint32_t) (text->bounds.size.width + 1.5);
  text->bounds.size.height = (uint32_t) (text->bounds.size.height + 1.5);
  text->bounds.origin.x = (int32_t) (text->bounds.origin.x + 0.5);
  text->bounds.origin.y = (int32_t) (text->bounds.origin.y + 0.5);

  text->width = text->bounds.size.width;

  CFRelease(string);
  CFRelease(attr_string);

  text_calculate_truncated_width(text, attributes);
  CFRelease(attributes);
}

static void text_destroy_line(struct text* text) {
  if (text->line.line) CFRelease(text->line.line);
  text->line.line = NULL;
}

static void badge_destroy_line(struct badge* badge) {
  if (badge->line.line) CFRelease(badge->line.line);
  badge->line.line = NULL;
}

static void badge_prepare_line(struct badge* badge) {
  const void *keys[] = { kCTFontAttributeName,
                         kCTForegroundColorFromContextAttributeName };

  if (badge->font.font_changed) {
    font_create_ctfont(&badge->font);
    badge->font.font_changed = false;
  }
  const void *values[] = { badge->font.ct_font, kCFBooleanTrue };
  CFDictionaryRef attributes = CFDictionaryCreate(NULL,
                                                  keys,
                                                  values,
                                                  array_count(keys),
                                                  &kCFTypeDictionaryKeyCallBacks,
                                                  &kCFTypeDictionaryValueCallBacks);

  CFStringRef string = CFStringCreateWithCString(NULL,
                                                 badge->string,
                                                 kCFStringEncodingUTF8);

  if (!string) string = CFStringCreateWithCString(NULL,
                                          "Warning: Malformed UTF-8 string",
                                          kCFStringEncodingUTF8             );

  if (!string) {
    badge->line.ascent = 0;
    badge->line.descent = 0;
    badge->bounds = CGRectZero;
    badge->width = 0.f;
    CFRelease(attributes);
    return;
  }

  CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL,
                                                               string,
                                                               attributes);

  if (!attr_string) {
    badge->line.ascent = 0;
    badge->line.descent = 0;
    badge->bounds = CGRectZero;
    badge->width = 0.f;
    if (string) CFRelease(string);
    CFRelease(attributes);
    return;
  }

  badge->line.line = CTLineCreateWithAttributedString(attr_string);

  if (!badge->line.line) {
    badge->line.ascent = 0;
    badge->line.descent = 0;
    badge->bounds = CGRectZero;
    badge->width = 0.f;
    CFRelease(string);
    CFRelease(attr_string);
    CFRelease(attributes);
    return;
  }

  CTLineGetTypographicBounds(badge->line.line,
                             &badge->line.ascent,
                             &badge->line.descent,
                             NULL                );

  badge->bounds = CTLineGetBoundsWithOptions(badge->line.line,
                                             kCTLineBoundsUseGlyphPathBounds);

  badge->bounds.size.width = (uint32_t)(badge->bounds.size.width + 1.5);
  badge->bounds.size.height = (uint32_t)(badge->bounds.size.height + 1.5);
  badge->bounds.origin.x = (int32_t)(badge->bounds.origin.x + 0.5);
  badge->bounds.origin.y = (int32_t)(badge->bounds.origin.y + 0.5);

  badge->width = badge->bounds.size.width;

  CFRelease(string);
  CFRelease(attr_string);
  CFRelease(attributes);
}

static bool badge_set_string(struct badge* badge, char* string, bool forced) {
  if (!string) return false;
  if (!forced && badge->string && strcmp(badge->string, string) == 0) {
    if (string != badge->string) free(string);
    return false;
  }

  if (badge->line.line) badge_destroy_line(badge);
  if (string != badge->string && badge->string) free(badge->string);
  badge->string = string;
  badge_prepare_line(badge);
  return true;
}

static bool badge_set_font(struct badge* badge, char* font_string, bool forced) {
  return font_set(&badge->font, font_string, forced);
}

static bool badge_set_color(struct badge* badge, uint32_t color) {
  return color_set_hex(&badge->color, color);
}

static bool badge_set_xoffset(struct badge* badge, int offset) {
  if (badge->x_offset == offset) return false;
  badge->x_offset = offset;
  return true;
}

static bool badge_set_yoffset(struct badge* badge, int offset) {
  if (badge->y_offset == offset) return false;
  badge->y_offset = offset;
  return true;
}

static bool badge_set_width(struct badge* badge, int width) {
  if (width < 0) {
    bool prev = badge->has_const_width;
    badge->has_const_width = false;
    return prev != badge->has_const_width;
  }

  if (badge->custom_width == width && badge->has_const_width) return false;
  badge->custom_width = width;
  badge->has_const_width = true;
  return true;
}

static bool badge_set_anchor(struct badge* badge, enum badge_anchor anchor) {
  if (badge->anchor == anchor) return false;
  badge->anchor = anchor;
  return true;
}

static uint32_t badge_get_text_width(struct badge* badge) {
  if (badge->font.font_changed)
    badge_set_string(badge, badge->string, true);

  return badge->width < 0 ? 0 : (uint32_t)badge->width;
}

static uint32_t badge_get_box_width(struct badge* badge) {
  if (badge->has_const_width)
    return badge->custom_width;

  return badge_get_text_width(badge);
}

static const char* badge_anchor_to_string(enum badge_anchor anchor) {
  switch (anchor) {
    case BADGE_ANCHOR_TOP_LEFT: return "top_left";
    case BADGE_ANCHOR_TOP_CENTER: return "top_center";
    case BADGE_ANCHOR_TOP_RIGHT: return "top_right";
    case BADGE_ANCHOR_CENTER_LEFT: return "center_left";
    case BADGE_ANCHOR_CENTER: return "center";
    case BADGE_ANCHOR_CENTER_RIGHT: return "center_right";
    case BADGE_ANCHOR_BOTTOM_LEFT: return "bottom_left";
    case BADGE_ANCHOR_BOTTOM_CENTER: return "bottom_center";
    case BADGE_ANCHOR_BOTTOM_RIGHT: return "bottom_right";
    default: return "invalid";
  }
}

static bool badge_parse_anchor(struct token token, enum badge_anchor* anchor) {
  if (token_equals(token, "top_left")) {
    *anchor = BADGE_ANCHOR_TOP_LEFT;
    return true;
  } else if (token_equals(token, "top_center") || token_equals(token, "top_centre")) {
    *anchor = BADGE_ANCHOR_TOP_CENTER;
    return true;
  } else if (token_equals(token, "top_right")) {
    *anchor = BADGE_ANCHOR_TOP_RIGHT;
    return true;
  } else if (token_equals(token, "center_left") || token_equals(token, "centre_left") || token_equals(token, "left_center") || token_equals(token, "left_centre")) {
    *anchor = BADGE_ANCHOR_CENTER_LEFT;
    return true;
  } else if (token_equals(token, "center") || token_equals(token, "centre")) {
    *anchor = BADGE_ANCHOR_CENTER;
    return true;
  } else if (token_equals(token, "center_right") || token_equals(token, "centre_right") || token_equals(token, "right_center") || token_equals(token, "right_centre")) {
    *anchor = BADGE_ANCHOR_CENTER_RIGHT;
    return true;
  } else if (token_equals(token, "bottom_left")) {
    *anchor = BADGE_ANCHOR_BOTTOM_LEFT;
    return true;
  } else if (token_equals(token, "bottom_center") || token_equals(token, "bottom_centre")) {
    *anchor = BADGE_ANCHOR_BOTTOM_CENTER;
    return true;
  } else if (token_equals(token, "bottom_right")) {
    *anchor = BADGE_ANCHOR_BOTTOM_RIGHT;
    return true;
  }

  return false;
}

static void badge_init(struct badge* badge) {
  memset(badge, 0, sizeof(struct badge));

  badge->drawing = false;
  badge->has_const_width = false;
  badge->align = POSITION_CENTER;
  badge->x_offset = 0;
  badge->y_offset = 0;
  badge->custom_width = 0;
  badge->width = 0.f;
  badge->anchor = BADGE_ANCHOR_BOTTOM_RIGHT;

  font_init(&badge->font);
  color_init(&badge->color, 0xffffffff);
  background_init(&badge->background);

  badge->string = NULL;
  badge_set_string(badge, string_copy(""), true);
}

static void badge_clear_pointers(struct badge* badge) {
  badge->string = NULL;
  badge->line.line = NULL;
  font_clear_pointers(&badge->font);
  background_clear_pointers(&badge->background);
}

static void badge_copy(struct badge* badge, struct badge* source) {
  badge->drawing = source->drawing;
  badge->has_const_width = source->has_const_width;
  badge->align = source->align;
  badge->x_offset = source->x_offset;
  badge->y_offset = source->y_offset;
  badge->custom_width = source->custom_width;
  badge->anchor = source->anchor;

  font_set_family(&badge->font, string_copy(source->font.family), true);
  font_set_style(&badge->font, string_copy(source->font.style), true);
  font_set_size(&badge->font, source->font.size);
  badge_set_string(badge, string_copy(source->string), true);
  color_set_hex(&badge->color, source->color.hex);
  badge->background = source->background;
  background_clear_pointers(&badge->background);
  image_copy(&badge->background.image, source->background.image.image_ref);
}

static void badge_calculate_bounds(struct badge* badge, CGRect parent) {
  if (!badge->drawing) return;

  uint32_t box_width = badge_get_box_width(badge);
  uint32_t text_width = badge_get_text_width(badge);
  uint32_t box_height = badge->background.overrides_height
                        ? badge->background.bounds.size.height
                        : badge->bounds.size.height;
  CGRect box = {{0, 0}, {box_width, box_height}};

  switch (badge->anchor) {
    case BADGE_ANCHOR_TOP_LEFT:
      box.origin.x = CGRectGetMinX(parent);
      box.origin.y = CGRectGetMaxY(parent) - box.size.height;
      break;
    case BADGE_ANCHOR_TOP_CENTER:
      box.origin.x = CGRectGetMidX(parent) - box.size.width / 2.f;
      box.origin.y = CGRectGetMaxY(parent) - box.size.height;
      break;
    case BADGE_ANCHOR_TOP_RIGHT:
      box.origin.x = CGRectGetMaxX(parent) - box.size.width;
      box.origin.y = CGRectGetMaxY(parent) - box.size.height;
      break;
    case BADGE_ANCHOR_CENTER_LEFT:
      box.origin.x = CGRectGetMinX(parent);
      box.origin.y = CGRectGetMidY(parent) - box.size.height / 2.f;
      break;
    case BADGE_ANCHOR_CENTER:
      box.origin.x = CGRectGetMidX(parent) - box.size.width / 2.f;
      box.origin.y = CGRectGetMidY(parent) - box.size.height / 2.f;
      break;
    case BADGE_ANCHOR_CENTER_RIGHT:
      box.origin.x = CGRectGetMaxX(parent) - box.size.width;
      box.origin.y = CGRectGetMidY(parent) - box.size.height / 2.f;
      break;
    case BADGE_ANCHOR_BOTTOM_LEFT:
      box.origin.x = CGRectGetMinX(parent);
      box.origin.y = CGRectGetMinY(parent);
      break;
    case BADGE_ANCHOR_BOTTOM_CENTER:
      box.origin.x = CGRectGetMidX(parent) - box.size.width / 2.f;
      box.origin.y = CGRectGetMinY(parent);
      break;
    case BADGE_ANCHOR_BOTTOM_RIGHT:
      box.origin.x = CGRectGetMaxX(parent) - box.size.width;
      box.origin.y = CGRectGetMinY(parent);
      break;
  }

  box.origin.x += badge->x_offset;
  box.origin.y += badge->y_offset;

  if (badge->align == POSITION_CENTER && badge->has_const_width)
    badge->bounds.origin.x = box.origin.x
                             + (CGFloat)((int)box_width - (int)text_width) / 2.f;
  else if (badge->align == POSITION_RIGHT && badge->has_const_width)
    badge->bounds.origin.x = box.origin.x
                             + (int)box_width - (int)text_width;
  else
    badge->bounds.origin.x = box.origin.x;

  badge->bounds.origin.y = CGRectGetMidY(box)
                           - ((badge->line.ascent
                               - badge->line.descent) / 2);

  if (badge->background.enabled)
    background_calculate_bounds(&badge->background,
                                CGRectGetMidX(box),
                                CGRectGetMidY(box),
                                box.size.width,
                                box.size.height);
}

static void badge_draw(struct badge* badge, CGContextRef context) {
  if (!badge->drawing) return;
  if (badge->font.font_changed)
    badge_set_string(badge, badge->string, true);
  if (badge->background.enabled)
    background_draw(&badge->background, context);
  if (!badge->line.line) return;

  CGContextSaveGState(context);
  CGContextSetRGBFillColor(context,
                           badge->color.r,
                           badge->color.g,
                           badge->color.b,
                           badge->color.a);
  CGContextSetTextPosition(context,
                           badge->bounds.origin.x,
                           badge->bounds.origin.y);
  CTLineDraw(badge->line.line, context);
  CGContextRestoreGState(context);
}

static void badge_destroy(struct badge* badge) {
  background_destroy(&badge->background);
  font_destroy(&badge->font);

  if (badge->string) free(badge->string);
  badge_destroy_line(badge);
  badge_clear_pointers(badge);
}

static void badge_serialize(struct badge* badge, char* indent, FILE* rsp) {
  char align[32] = { 0 };
  switch (badge->align) {
    case POSITION_LEFT:
      snprintf(align, 32, "left");
      break;
    case POSITION_RIGHT:
      snprintf(align, 32, "right");
      break;
    case POSITION_CENTER:
      snprintf(align, 32, "center");
      break;
    default:
      snprintf(align, 32, "invalid");
      break;
  }

  fprintf(rsp, "%s\"value\": \"%s\",\n"
               "%s\"drawing\": \"%s\",\n"
               "%s\"color\": \"0x%x\",\n"
               "%s\"x_offset\": %d,\n"
               "%s\"y_offset\": %d,\n"
               "%s\"font\": \"%s:%s:%.2f\",\n"
               "%s\"width\": %d,\n"
               "%s\"align\": \"%s\",\n"
               "%s\"anchor\": \"%s\",\n"
               "%s\"background\": {\n",
               indent, badge->string,
               indent, format_bool(badge->drawing),
               indent, badge->color.hex,
               indent, badge->x_offset,
               indent, badge->y_offset,
               indent, badge->font.family, badge->font.style, badge->font.size,
               indent, badge->has_const_width ? badge->custom_width : -1,
               indent, align,
               indent, badge_anchor_to_string(badge->anchor),
               indent);

  char deeper_indent[strlen(indent) + 2];
  snprintf(deeper_indent, strlen(indent) + 2, "%s\t", indent);
  background_serialize(&badge->background, deeper_indent, rsp, true);
  fprintf(rsp, "\n%s}", indent);
}

static bool badge_parse_sub_domain(struct badge* badge, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_STRING)) {
    return badge_set_string(badge, token_to_string(get_token(&message)), false);
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    bool prev = badge->drawing;
    badge->drawing = evaluate_boolean_state(get_token(&message), badge->drawing);
    return prev != badge->drawing;
  } else if (token_equals(property, PROPERTY_FONT)) {
    needs_refresh = badge_set_font(badge, string_copy(message), false);
  } else if (token_equals(property, PROPERTY_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(badge_set_color,
                  badge,
                  badge->color.hex,
                  token_to_int(token));
  } else if (token_equals(property, PROPERTY_XOFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(badge_set_xoffset,
            badge,
            badge->x_offset,
            token_to_int(token));
  } else if (token_equals(property, PROPERTY_YOFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(badge_set_yoffset,
            badge,
            badge->y_offset,
            token_to_int(token));
  } else if (token_equals(property, PROPERTY_WIDTH)) {
    struct token token = get_token(&message);
    ANIMATE(badge_set_width,
            badge,
            badge_get_box_width(badge),
            token_to_int(token));
  } else if (token_equals(property, PROPERTY_ALIGN)) {
    char prev = badge->align;
    badge->align = get_token(&message).text[0];
    return prev != badge->align;
  } else if (token_equals(property, PROPERTY_ANCHOR)) {
    struct token token = get_token(&message);
    enum badge_anchor anchor;
    if (!badge_parse_anchor(token, &anchor)) {
      respond(rsp,
              "[!] Badge: Invalid anchor '%.*s' "
              "(expected top_left|top_center|top_right|center_left|center|center_right|bottom_left|bottom_center|bottom_right)\n",
              token.length,
              token.text);
      return false;
    }
    needs_refresh = badge_set_anchor(badge, anchor);
  } else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text,
                                                              '.'           );
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
      struct token entry = { key_value_pair.value,
                             strlen(key_value_pair.value) };

      if (token_equals(subdom, SUB_DOMAIN_FONT))
        return font_parse_sub_domain(&badge->font, rsp, entry, message);
      else if (token_equals(subdom, SUB_DOMAIN_COLOR))
        return color_parse_sub_domain(&badge->color, rsp, entry, message);
      else if (token_equals(subdom, SUB_DOMAIN_BACKGROUND))
        return background_parse_sub_domain(&badge->background, rsp, entry, message);
      else
        respond(rsp, "[!] Badge: Invalid subdomain '%s'\n", subdom.text);
    } else {
      respond(rsp, "[!] Badge: Invalid property '%s'\n", property.text);
    }
  }

  return needs_refresh;
}

bool text_set_max_chars(struct text* text, uint32_t max_chars) {
  if (text->max_chars == max_chars) return false;
  text->max_chars = max_chars;
  if (strlen(text->string) > text->max_chars) {
    text_set_string(text, text->string, true);
  }
  return strlen(text->string) > text->max_chars;
}

bool text_set_string(struct text* text, char* string, bool forced) {
  if (!string) return false;
  if (!forced && text->string && strcmp(text->string, string) == 0) { 
    if (!(string == text->string)) free(string);
    return false; 
  }
  if (text->line.line) text_destroy_line(text);
  if (string != text->string && text->string) free(text->string);
  text->string = string;
  text_prepare_line(text);
  return true;
}

void text_copy(struct text* text, struct text* source) {
  font_set_family(&text->font, string_copy(source->font.family), true);
  font_set_style(&text->font, string_copy(source->font.style), true);
  font_set_size(&text->font, source->font.size);
  text_set_string(text, string_copy(source->string), true);
  badge_copy(&text->badge, &source->badge);
}

bool text_set_font(struct text* text, char* font_string, bool forced) {
  bool changed = font_set(&text->font, font_string, forced);
  return changed;
}

void text_init(struct text* text) {
  text->drawing = true;
  text->highlight = false;
  text->has_const_width = false;
  text->custom_width = 0;
  text->padding_left = 0;
  text->padding_right = 0;
  text->y_offset = 0;
  text->x_offset = 0;
  text->max_chars = 0;
  text->align = POSITION_LEFT;
  text->scroll = 0.f;
  text->scroll_duration = 100;

  shadow_init(&text->shadow);
  background_init(&text->background);
  font_init(&text->font);
  color_init(&text->color, 0xffffffff);
  color_init(&text->highlight_color, 0xff000000);
  badge_init(&text->badge);

  text->string = string_copy("");
  text_set_string(text, text->string, false);
}

static bool text_set_color(struct text* text, uint32_t color) {
  return color_set_hex(&text->color, color);
}

static bool text_set_highlight_color(struct text* text, uint32_t color) {
  return color_set_hex(&text->highlight_color, color);
}

static bool text_set_xoffset(struct text* text, int offset) {
  if (text->x_offset == offset) return false;
  text->x_offset = offset;
  return true;
}

static bool text_set_padding_left(struct text* text, int padding) {
  if (text->padding_left == padding) return false;
  text->padding_left = padding;
  return true;
}

static bool text_set_padding_right(struct text* text, int padding) {
  if (text->padding_right == padding) return false;
  text->padding_right = padding;
  return true;
}

static bool text_set_yoffset(struct text* text, int offset) {
  if (text->y_offset == offset) return false;
  text->y_offset = offset;
  return true;
}

static bool text_set_scroll_duration(struct text* text, int duration) {
  if (duration < 0) return false;
  text->scroll_duration = duration;
  return false;
}

static bool text_set_width(struct text* text, int width) {
  if (width < 0) {
    bool prev = text->has_const_width;
    text->has_const_width = false;
    return prev != text->has_const_width;
  }

  if (text->custom_width == width && text->has_const_width) return false;
  text->custom_width = width;
  text->has_const_width = true;
  return true;
}

void text_clear_pointers(struct text* text) {
  text->string = NULL;
  text->line.line = NULL;
  background_clear_pointers(&text->background);
  font_clear_pointers(&text->font);
  badge_clear_pointers(&text->badge);
}

uint32_t text_get_length(struct text* text, bool override) {
  if (!text->drawing) return 0;

  if (text->font.font_changed) {
    text_set_string(text, text->string, true);
  }

  int len = text->width + text->padding_left + text->padding_right;
  if ((!text->has_const_width || override)
      && text->background.enabled
      && text->background.image.enabled) {
    CGSize image_size = image_get_size(&text->background.image);
    if (image_size.width > len) {
      return image_size.width;
    }
  }

  if (text->has_const_width && !override) return text->custom_width;
  return (len < 0 ? 0 : len);
}

uint32_t text_get_height(struct text* text) {
  return text->drawing ? text->bounds.size.height : 0;
}

void text_destroy(struct text* text) {
  background_destroy(&text->background);
  font_destroy(&text->font);
  badge_destroy(&text->badge);

  if (text->string) free(text->string);
  text_destroy_line(text);
  text_clear_pointers(text);
}

void text_calculate_bounds(struct text* text, uint32_t x, uint32_t y) {
  if (text->align == POSITION_CENTER && text->has_const_width)
    text->bounds.origin.x = x + text->x_offset + ((int)text->custom_width
                                 - (int)text_get_length(text, true)) / 2.f;
  else if (text->align == POSITION_RIGHT && text->has_const_width)
    text->bounds.origin.x = x + text->x_offset + (int)text->custom_width
                            - (int)text_get_length(text, true);
  else
    text->bounds.origin.x = x + text->x_offset;

  text->bounds.origin.y =(uint32_t)(y - ((text->line.ascent
                                          - text->line.descent) / 2));

  if (text->background.enabled) {
    uint32_t height = text->background.overrides_height
                      ? text->background.bounds.size.height
                      : text->bounds.size.height;

    background_calculate_bounds(&text->background,
                                x + text->x_offset,
                                y,
                                text_get_length(text, false),
                                height                       );
  }

  CGRect badge_parent;
  if (text->background.enabled) {
    badge_parent = text->background.bounds;
    badge_parent.origin.x += text->background.x_offset;
    badge_parent.origin.y += text->background.y_offset;
  } else {
    badge_parent = text->bounds;
    badge_parent.origin.x += text->padding_left;
    badge_parent.origin.y += text->y_offset;
  }

  badge_calculate_bounds(&text->badge, badge_parent);
}

bool text_set_scroll(struct text* text, float scroll) {
  if (text->scroll == scroll) return false;
  text->scroll = scroll;
  return true;
}

bool text_animate_scroll(struct text* text) {
  if (text->max_chars == 0) return false;
  if (text->scroll != 0) return false;
  if (text->has_const_width && text->custom_width < text->width) return false;
  if (text->width == 0 || text->width == text->bounds.size.width) return false;

  g_bar_manager.animator.duration = text->scroll_duration
                                    * (text->bounds.size.width / text->width);
  g_bar_manager.animator.interp_function = INTERP_FUNCTION_LINEAR;

  bool needs_refresh = false;
  ANIMATE_FLOAT(text_set_scroll,
                text,
                text->scroll,
                max(text->bounds.size.width, 0));

  struct animation* animation = animation_create();
  float initial_value = text->scroll;
  float final_value = -max(text->width, 0);

  animation_setup(animation,
                  (void*)text,
                  (bool (*)(void*, int))text_set_scroll,
                  *(int*)&initial_value,
                  *(int*)&final_value,
                  0,
                  INTERP_FUNCTION_LINEAR );
  animation->as_float = true;
  animator_add(&g_bar_manager.animator, animation);

  g_bar_manager.animator.duration = text->scroll_duration;
  ANIMATE_FLOAT(text_set_scroll, text, text->scroll, 0);

  g_bar_manager.animator.duration = 0;
  g_bar_manager.animator.interp_function = '\0';

  return needs_refresh;
}

void text_draw(struct text* text, CGContextRef context) {
  if (!text->drawing) return;
  if (text->background.enabled)
    background_draw(&text->background, context);

  CGContextSaveGState(context);
  if (text->max_chars > 0) {
    CGMutablePathRef path = CGPathCreateMutable();
    CGRect bounds = text->bounds;
    bounds.size.width = text->width;
    bounds.origin.x += text->padding_left;
    bounds.origin.y = -9999.f;
    bounds.size.height = 2.f*9999.f;

    CGPathAddRect(path, NULL, bounds);

    CGContextAddPath(context, path);
    CGContextClip(context);
    CFRelease(path);
  }

  if (text->shadow.enabled) {
    CGContextSetRGBFillColor(context,
                             text->shadow.color.r,
                             text->shadow.color.g,
                             text->shadow.color.b,
                             text->shadow.color.a );

    CGRect bounds = shadow_get_bounds(&text->shadow, text->bounds);
    CGContextSetTextPosition(context,
                             bounds.origin.x + text->padding_left,
                             bounds.origin.y + text->y_offset     );
    CTLineDraw(text->line.line, context);
  }

  struct color color = text->highlight ? text->highlight_color : text->color;
  CGContextSetRGBFillColor(context, color.r, color.g, color.b, color.a);

  CGContextSetTextPosition(context,
                           text->bounds.origin.x + text->padding_left
                           - text->scroll,
                           text->bounds.origin.y + text->y_offset    );
  CTLineDraw(text->line.line, context);
  CGContextRestoreGState(context);
}

void text_draw_badge(struct text* text, CGContextRef context) {
  badge_draw(&text->badge, context);
}

void text_serialize(struct text* text, char* indent, FILE* rsp) {
  char align[32] = { 0 };
  switch (text->align) {
    case POSITION_LEFT:
      snprintf(align, 32, "left");
      break;
    case POSITION_RIGHT:
      snprintf(align, 32, "right");
      break;
    case POSITION_CENTER:
      snprintf(align, 32, "center");
      break;
    case POSITION_BOTTOM:
      snprintf(align, 32, "bottom");
      break;
    case POSITION_TOP:
      snprintf(align, 32, "top");
      break;
    default:
      snprintf(align, 32, "invalid");
      break;
  }

  fprintf(rsp, "%s\"value\": \"%s\",\n"
               "%s\"drawing\": \"%s\",\n"
               "%s\"highlight\": \"%s\",\n"
               "%s\"color\": \"0x%x\",\n"
               "%s\"highlight_color\": \"0x%x\",\n"
               "%s\"x_offset\": %d,\n"
               "%s\"padding_left\": %d,\n"
               "%s\"padding_right\": %d,\n"
               "%s\"y_offset\": %d,\n"
               "%s\"font\": \"%s:%s:%.2f\",\n"
               "%s\"width\": %d,\n"
               "%s\"scroll_duration\": %d,\n"
               "%s\"align\": \"%s\",\n"
               "%s\"background\": {\n",
               indent, text->string,
               indent, format_bool(text->drawing),
               indent, format_bool(text->highlight),
               indent, text->color.hex,
               indent, text->highlight_color.hex,
               indent, text->x_offset,
               indent, text->padding_left,
               indent, text->padding_right,
               indent, text->y_offset,
               indent, text->font.family, text->font.style, text->font.size,
               indent, text->custom_width,
               indent, text->scroll_duration,
               indent, align, indent                                        );

  char deeper_indent[strlen(indent) + 2];
  snprintf(deeper_indent, strlen(indent) + 2, "%s\t", indent);
  background_serialize(&text->background, deeper_indent, rsp, true);

  fprintf(rsp, "\n%s},\n%s\"badge\": {\n", indent, indent);
  badge_serialize(&text->badge, deeper_indent, rsp);
  fprintf(rsp, "\n%s},\n%s\"shadow\": {\n", indent, indent);
  shadow_serialize(&text->shadow, deeper_indent, rsp);
  fprintf(rsp, "\n%s}", indent);
}

bool text_parse_sub_domain(struct text* text, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(text_set_color,
                  text,
                  text->color.hex,
                  token_to_int(token));
  }
  else if (token_equals(property, PROPERTY_HIGHLIGHT)) {
    bool highlight = evaluate_boolean_state(get_token(&message),
                                             text->highlight    );
    if (g_bar_manager.animator.duration > 0) {
      if (text->highlight && !highlight) {
        animator_cancel(&g_bar_manager.animator,
                        text,
                        (animator_function*)text_set_color);

        uint32_t target = text->color.hex;
        text_set_color(text, text->highlight_color.hex);

        ANIMATE_BYTES(text_set_color,
                      text,
                      text->color.hex,
                      target          );
      }
      else if (!text->highlight && highlight) {
        animator_cancel(&g_bar_manager.animator,
                        text,
                        (animator_function*)text_set_highlight_color);

        uint32_t target = text->highlight_color.hex;
        text_set_highlight_color(text, text->color.hex);

        ANIMATE_BYTES(text_set_highlight_color,
                      text,
                      text->highlight_color.hex,
                      target                    );
      }
    }

    needs_refresh = text->highlight != highlight;
    text->highlight = highlight;
  } else if (token_equals(property, PROPERTY_FONT))
    needs_refresh = text_set_font(text, string_copy(message), false);
  else if (token_equals(property, PROPERTY_HIGHLIGHT_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(text_set_highlight_color,
                  text,
                  text->highlight_color.hex,
                  token_to_int(token)       );

  } else if (token_equals(property, PROPERTY_XOFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(text_set_xoffset,
            text,
            text->x_offset,
            token_to_int(token));
  } else if (token_equals(property, PROPERTY_PADDING_LEFT)) {
    struct token token = get_token(&message);
    ANIMATE(text_set_padding_left,
            text,
            text->padding_left,
            token_to_int(token)  );

  } else if (token_equals(property, PROPERTY_PADDING_RIGHT)) {
    struct token token = get_token(&message);
    ANIMATE(text_set_padding_right,
            text,
            text->padding_right,
            token_to_int(token)    );

  } else if (token_equals(property, PROPERTY_YOFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(text_set_yoffset,
            text,
            text->y_offset,
            token_to_int(token));

  } else if (token_equals(property, PROPERTY_SCROLL_DURATION)) {
    struct token token = get_token(&message);
    text_set_scroll_duration(text, token_to_int(token));
  } else if (token_equals(property, PROPERTY_WIDTH)) {
    struct token token = get_token(&message);
    if (token_equals(token, ARGUMENT_DYNAMIC)) {
      ANIMATE(text_set_width,
              text,
              text->custom_width,
              text_get_length(text, true));

      struct animation* animation = animation_create();
      animation_setup(animation,
                      text,
                      (bool (*)(void*, int))&text_set_width,
                      text->custom_width,
                      -1,
                      0,
                      INTERP_FUNCTION_LINEAR               );
      animator_add(&g_bar_manager.animator, animation);
    }
    else {
      ANIMATE(text_set_width,
              text,
              text_get_length(text, false),
              token_to_int(token)          );
    }
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    bool prev = text->drawing;
    text->drawing = evaluate_boolean_state(get_token(&message), text->drawing);
    return prev != text->drawing;
  } else if (token_equals(property, PROPERTY_ALIGN)) {
    char prev = text->align;
    text->align = get_token(&message).text[0];
    return prev != text->align;
  } else if (token_equals(property, PROPERTY_STRING)) {
    uint32_t pre_width = text_get_length(text, false);
    bool changed = text_set_string(text,
                                   token_to_string(get_token(&message)),
                                   false                                );

    if (changed
        && g_bar_manager.animator.duration > 0) {
      uint32_t post_width = text_get_length(text, false);
      if (post_width != pre_width) {
        text_set_width(text, pre_width);
        ANIMATE(text_set_width, text, pre_width, post_width);

        struct animation* animation = animation_create();
        animation_setup(animation,
                        text,
                        (bool (*)(void*, int))&text_set_width,
                        text->custom_width,
                        -1,
                        0,
                        INTERP_FUNCTION_LINEAR               );
        animator_add(&g_bar_manager.animator, animation);
      }
    }

    return changed;
  } else if (token_equals(property, SUB_DOMAIN_BADGE)) {
    struct token dummy = { PROPERTY_STRING, strlen(PROPERTY_STRING) };
    return badge_parse_sub_domain(&text->badge,
                                  rsp,
                                  dummy,
                                  message       );
  } else if (token_equals(property, PROPERTY_MAX_CHARS)) {
    return text_set_max_chars(text, token_to_int(get_token(&message)));
  }
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text,
                                                              '.'           );
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
      struct token entry = { key_value_pair.value,
                             strlen(key_value_pair.value) };
      if (token_equals(subdom, SUB_DOMAIN_BACKGROUND))
        return background_parse_sub_domain(&text->background,
                                           rsp,
                                           entry,
                                           message           );
      else if (token_equals(subdom, SUB_DOMAIN_SHADOW))
        return shadow_parse_sub_domain(&text->shadow, rsp, entry, message);
      else if (token_equals(subdom, SUB_DOMAIN_BADGE))
        return badge_parse_sub_domain(&text->badge, rsp, entry, message);
      else if (token_equals(subdom, SUB_DOMAIN_FONT))
        return font_parse_sub_domain(&text->font, rsp, entry, message);
      else if (token_equals(subdom, SUB_DOMAIN_COLOR))
        return color_parse_sub_domain(&text->color, rsp, entry, message);
      else if (token_equals(subdom, SUB_DOMAIN_HIGHLIGHT_COLOR))
        return color_parse_sub_domain(&text->highlight_color,
                                      rsp,
                                      entry,
                                      message);
      else
        respond(rsp, "[!] Text: Invalid subdomain '%s' \n", subdom.text);
    }
    else {
      respond(rsp, "[!] Text: Invalid property '%s'\n", property.text);
    }
  }

  return needs_refresh;
}
