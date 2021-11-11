#include "text.h"
#include "misc/helpers.h"
#include <stdint.h>

static CTFontRef text_create_font(char *cstring) {
  float size = 10.0f;
  char font_properties[2][255] = { {}, {} };
  sscanf(cstring, "%254[^:]:%254[^:]:%f", font_properties[0], font_properties[1], &size);
  CFStringRef font_family_name = CFStringCreateWithCString(NULL, font_properties[0], kCFStringEncodingUTF8);
  CFStringRef font_style_name = CFStringCreateWithCString(NULL, font_properties[1], kCFStringEncodingUTF8);
  CFNumberRef font_size = CFNumberCreate(NULL, kCFNumberFloat32Type, &size);

  const void *keys[] = { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute, kCTFontSizeAttribute };
  const void *values[] = { font_family_name, font_style_name, font_size };
  CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
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

  text->color = rgba_color_from_hex(0xffffffff);

  text->font = NULL;
  text->string = string_copy("");
  text->font_name = string_copy("Hack Nerd Font:Bold:14.0");
  text_set_font(text, text->font_name, true);
  text_set_string(text, text->string, false);
}

void text_prepare_line(struct text_line* text_line, CTFontRef font, char* cstring, struct rgba_color color) {
  const void *keys[] = { kCTFontAttributeName, kCTForegroundColorFromContextAttributeName };
  const void *values[] = { font, kCFBooleanTrue };
  CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFStringRef string = CFStringCreateWithCString(NULL, cstring, kCFStringEncodingUTF8);
  if (!string) string = CFStringCreateWithCString(NULL, "Warning: Malformed UTF-8 string", kCFStringEncodingUTF8);
  CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL, string, attributes);
  text_line->line = CTLineCreateWithAttributedString(attr_string);

  CTLineGetTypographicBounds(text_line->line, &text_line->ascent, &text_line->descent, NULL);
  text_line->bounds = CTLineGetBoundsWithOptions(text_line->line, kCTLineBoundsUseGlyphPathBounds);
  text_line->bounds.size.width = (uint32_t) (text_line->bounds.size.width + 0.5);
  text_line->bounds.origin.x = (uint32_t) (text_line->bounds.origin.x + 0.5);
  text_line->color = color;

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
  if (string != text->string && !text->string) free(text->string);
  text->string = string;
  text_prepare_line(&text->line, text->font, text->string, text->highlight ? text->highlight_color : text->color);
  return true;
}

bool text_set_color(struct text* text, uint32_t color) {
  text->color = rgba_color_from_hex(color);
  return text_update_color(text);
}

bool text_set_font(struct text* text, char* font_string, bool forced) {
  if (!font_string) return false;
  if (!forced && text->font_name && strcmp(text->font_name, font_string) == 0) { free(font_string); return false; }
  if (text->font) CFRelease(text->font);

  text->font = text_create_font(font_string);
  text->font_name = font_string;
  return text_set_string(text, text->string, true);
}

bool text_update_color(struct text* text) {
  struct rgba_color target_color = text->highlight ? text->highlight_color : text->color;
  if (text->line.color.r == target_color.r 
      && text->line.color.g == target_color.g 
      && text->line.color.b == target_color.b 
      && text->line.color.a == target_color.a) return false;
  text->line.color = target_color;
  return true;
}

void text_clear_pointers(struct text* text) {
  text->string = NULL;
  text->font_name = NULL;
  text->font = NULL;
  text->line.line = NULL;
}

uint32_t text_get_length(struct text* text) {
  if (!text->drawing) return 0;
  if (text->has_const_width) return text->custom_width;
  return (text->line.bounds.size.width + text->padding_left + text->padding_right) > 0 ? (text->line.bounds.size.width + text->padding_left + text->padding_right) : 0;
}

uint32_t text_get_height(struct text* text) {
  return text->line.bounds.size.height;
}

void text_destroy_line(struct text* text) {
  if (text->line.line) CFRelease(text->line.line);
  text->line.line = NULL;
}

void text_destroy(struct text* text) {
  if (text->string) free(text->string);
  if (text->font_name) free(text->font_name);
  if (text->font) CFRelease(text->font);
  text_destroy_line(text);
  text_clear_pointers(text);
}

void text_draw(struct text* text, CGPoint origin, CGContextRef context) {
  if (!text->drawing) return;
  CGContextSetRGBFillColor(context, text->line.color.r, text->line.color.g, text->line.color.b, text->line.color.a);
  CGContextSetTextPosition(context, origin.x + text->padding_left, origin.y + text->y_offset);
  CTLineDraw(text->line.line, context);
}

static bool text_parse_sub_domain(struct text* text, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_COLOR))
    return text_set_color(text, token_to_uint32t(get_token(&message)));
  else if (token_equals(property, PROPERTY_HIGHLIGHT)) {
    text->highlight = evaluate_boolean_state(get_token(&message), text->highlight);
    return text_update_color(text);
  } else if (token_equals(property, PROPERTY_FONT))
    return text_set_font(text, string_copy(message), false);
  else if (token_equals(property, PROPERTY_HIGHLIGHT_COLOR)) {
    text->highlight_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
    return text_update_color(text);
  } else if (token_equals(property, PROPERTY_PADDING_LEFT)) {
    text->padding_left = token_to_int(get_token(&message));
    return true;
  } else if (token_equals(property, PROPERTY_PADDING_RIGHT)) {
    text->padding_right = token_to_int(get_token(&message));
    return true;
  } else if (token_equals(property, PROPERTY_YOFFSET)) {
    text->y_offset = token_to_int(get_token(&message));
    return true;
  } else if (token_equals(property, PROPERTY_WIDTH)) {
    struct token token = get_token(&message);
    if (token_equals(token, "dynamic"))
      text->has_const_width = false;
    else {
      text->has_const_width = true;
      text->custom_width = token_to_uint32t(token);
    }
    return true;
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    text->drawing = evaluate_boolean_state(get_token(&message), text->drawing);
    return true;
  } 
  else {
    fprintf(rsp, "Unknown property: %s \n", property.text);
    printf("Unknown property: %s \n", property.text);
  }
  return false;
}
