#include "text.h"
#include "bar_manager.h"

static CTFontRef text_create_font(char *cstring) {
  float size = 10.0f;
  char font_properties[2][255] = { {}, {} };
  sscanf(cstring,
         "%254[^:]:%254[^:]:%f",
         font_properties[0],
         font_properties[1],
         &size                  );

  CFStringRef font_family_name = CFStringCreateWithCString(NULL,
                                                           font_properties[0],
                                                           kCFStringEncodingUTF8);

  CFStringRef font_style_name = CFStringCreateWithCString(NULL,
                                                          font_properties[1],
                                                          kCFStringEncodingUTF8);

  CFNumberRef font_size = CFNumberCreate(NULL, kCFNumberFloat32Type, &size);

  const void *keys[] = { kCTFontFamilyNameAttribute,
                         kCTFontStyleNameAttribute,
                         kCTFontSizeAttribute       };

  const void *values[] = { font_family_name, font_style_name, font_size };
  CFDictionaryRef attributes = CFDictionaryCreate(NULL,
                                                  keys,
                                                  values,
                                                  array_count(keys),
                                                  &kCFTypeDictionaryKeyCallBacks,
                                                  &kCFTypeDictionaryValueCallBacks);

  CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
  CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);

  CFRelease(descriptor);
  CFRelease(attributes);
  CFRelease(font_size);
  CFRelease(font_style_name);
  CFRelease(font_family_name);

  return font;
}

void text_init(struct text* text) {
  text->drawing = true;
  text->highlight = false;
  text->has_const_width = false;
  text->custom_width = 0;
  text->padding_left = 0;
  text->padding_right = 0;
  text->y_offset = 0;
  text->align = POSITION_LEFT;

  text->color = rgba_color_from_hex(0xffffffff);
  text->highlight_color = rgba_color_from_hex(0xff000000);

  text->font = NULL;
  text->string = string_copy("");
  text->font_name = string_copy("Hack Nerd Font:Bold:14.0");
  text_set_font(text, text->font_name, true);
  text_set_string(text, text->string, false);
  shadow_init(&text->shadow);
  background_init(&text->background);
}

void text_prepare_line(struct text* text) {
  const void *keys[] = { kCTFontAttributeName,
                         kCTForegroundColorFromContextAttributeName };

  const void *values[] = { text->font, kCFBooleanTrue };
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
  text->bounds.size.height = (uint32_t) (text->bounds.size.height + 0.5);
  text->bounds.origin.x = (uint32_t) (text->bounds.origin.x + 0.5);
  text->line.color = text->highlight ? text->highlight_color : text->color;

  CFRelease(string);
  CFRelease(attributes);
  CFRelease(attr_string);
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

bool text_set_color(struct text* text, uint32_t color) {
  text->color = rgba_color_from_hex(color);
  return text_update_color(text);
}

bool text_set_hightlight_color(struct text* text, uint32_t color) {
  text->highlight_color = rgba_color_from_hex(color);
  return text_update_color(text);
}

bool text_set_font(struct text* text, char* font_string, bool forced) {
  if (!font_string) return false;
  if (!forced && text->font_name
      && strcmp(text->font_name, font_string) == 0) {
    free(font_string);
    return false;
  }

  if (font_string != text->font_name && text->font_name)
    free(text->font_name);
  if (text->font) CFRelease(text->font);

  text->font = text_create_font(font_string);
  text->font_name = font_string;
  return text_set_string(text, text->string, true);
}

bool text_set_padding_left(struct text* text, int padding) {
  if (text->padding_left == padding) return false;
  text->padding_left = padding;
  return true;
}

bool text_set_padding_right(struct text* text, int padding) {
  if (text->padding_right == padding) return false;
  text->padding_right = padding;
  return true;
}

bool text_set_yoffset(struct text* text, int offset) {
  if (text->y_offset == offset) return false;
  text->y_offset = offset;
  return true;
}

bool text_set_width(struct text* text, int width) {
  if (text->custom_width == width && text->has_const_width) return false;
  text->custom_width = width;
  text->has_const_width = true;
  return true;
}

bool text_update_color(struct text* text) {
  struct rgba_color target_color = text->highlight
                                   ? text->highlight_color
                                   : text->color;

  if (text->line.color.r == target_color.r 
      && text->line.color.g == target_color.g 
      && text->line.color.b == target_color.b 
      && text->line.color.a == target_color.a) {
    return false;
  }

  text->line.color = target_color;
  return true;
}

void text_clear_pointers(struct text* text) {
  text->string = NULL;
  text->font_name = NULL;
  text->font = NULL;
  text->line.line = NULL;
  background_clear_pointers(&text->background);
}

uint32_t text_get_length(struct text* text, bool override) {
  if (!text->drawing) return 0;
  int len = text->bounds.size.width + text->padding_left + text->padding_right;
  if ((!text->has_const_width || override)
      && text->background.enabled
      && text->background.image.enabled
      && text->background.image.bounds.size.width > len
      && !text->has_const_width                        ) {
    return text->background.image.bounds.size.width;
  }

  if (text->has_const_width && !override) return text->custom_width;
  return (len < 0 ? 0 : len);
}

uint32_t text_get_height(struct text* text) {
  return text->bounds.size.height;
}

void text_destroy_line(struct text* text) {
  if (text->line.line) CFRelease(text->line.line);
  text->line.line = NULL;
}

void text_destroy(struct text* text) {
  background_destroy(&text->background);
  if (text->string) free(text->string);
  if (text->font_name) free(text->font_name);
  if (text->font) CFRelease(text->font);
  text_destroy_line(text);
  text_clear_pointers(text);
}

void text_calculate_bounds(struct text* text, uint32_t x, uint32_t y) {
  if (text->align == POSITION_CENTER && text->has_const_width)
    text->bounds.origin.x = (int)x + ((int)text->custom_width
                                 - (int)text_get_length(text, true)) / 2;
  else if (text->align == POSITION_RIGHT && text->has_const_width)
    text->bounds.origin.x = (int)x + (int)text->custom_width
                            - (int)text_get_length(text, true);
  else
    text->bounds.origin.x = x;

  text->bounds.origin.y =(uint32_t)(y - ((text->line.ascent
                                          - text->line.descent) / 2));

  if (text->background.enabled) {
    text->background.bounds.size.width = text_get_length(text, false);
    text->background.bounds.size.height = text->background.overrides_height
                                          ? text->background.bounds.size.height
                                          : text->bounds.size.height;
    background_calculate_bounds(&text->background, x, y);
  }
}

void text_draw(struct text* text, CGContextRef context) {
  if (!text->drawing) return;
  if (text->background.enabled)
    background_draw(&text->background, context);

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

  CGContextSetRGBFillColor(context,
                           text->line.color.r,
                           text->line.color.g,
                           text->line.color.b,
                           text->line.color.a );

  CGContextSetTextPosition(context,
                           text->bounds.origin.x + text->padding_left,
                           text->bounds.origin.y + text->y_offset     );
  CTLineDraw(text->line.line, context);
}

bool text_parse_sub_domain(struct text* text, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(text_set_color,
                  text,
                  hex_from_rgba_color(text->color),
                  token_to_int(token)              );
  }
  else if (token_equals(property, PROPERTY_HIGHLIGHT)) {
    bool highlight = evaluate_boolean_state(get_token(&message),
                                             text->highlight    );
    if (g_bar_manager.animator.duration > 0) {
      if (text->highlight && !highlight) {
        uint32_t target = hex_from_rgba_color(text->color);
        text_set_color(text, hex_from_rgba_color(text->highlight_color));

        ANIMATE_BYTES(text_set_color,
                      text,
                      hex_from_rgba_color(text->color),
                      target                           );
      }
      else if (!text->highlight && highlight) {
        uint32_t target = hex_from_rgba_color(text->highlight_color);
        text_set_hightlight_color(text, hex_from_rgba_color(text->color));

        ANIMATE_BYTES(text_set_hightlight_color,
                      text,
                      hex_from_rgba_color(text->highlight_color),
                      target                                     );
      }
    }

    text->highlight = highlight;
    needs_refresh = text_update_color(text);
  } else if (token_equals(property, PROPERTY_FONT))
    needs_refresh = text_set_font(text, string_copy(message), false);
  else if (token_equals(property, PROPERTY_HIGHLIGHT_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(text_set_hightlight_color,
                  text,
                  hex_from_rgba_color(text->highlight_color),
                  token_to_int(token)                        );

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

  } else if (token_equals(property, PROPERTY_WIDTH)) {
    struct token token = get_token(&message);
    if (token_equals(token, ARGUMENT_DYNAMIC)) {
      if (text->has_const_width) {
        needs_refresh = text->has_const_width;
        text->has_const_width = false;
      }
    }
    else {
      ANIMATE(text_set_width,
              text,
              text->custom_width,
              token_to_int(token));
    }
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    bool prev = text->drawing;
    text->drawing = evaluate_boolean_state(get_token(&message), text->drawing);
    return prev != text->drawing;
  } else if (token_equals(property, PROPERTY_ALIGN)) {
    char prev = text->align;
    text->align = get_token(&message).text[0];
    return prev != text->align;
  } 
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text, '.');
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
      struct token entry = { key_value_pair.value, strlen(key_value_pair.value) };
      if (token_equals(subdom, SUB_DOMAIN_BACKGROUND))
        return background_parse_sub_domain(&text->background, rsp, entry, message);
      else if (token_equals(subdom, SUB_DOMAIN_SHADOW))
        return shadow_parse_sub_domain(&text->shadow, rsp, entry, message);
      else {
        respond(rsp, "[!] Text: Invalid subdomain '%s' \n", subdom.text);
      }
    }
    else {
      respond(rsp, "[!] Text: Invalid property '%s'\n", property.text);
    }
  }

  return needs_refresh;
}
