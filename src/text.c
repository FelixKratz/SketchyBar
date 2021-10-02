#include "text.h"
#include "misc/helpers.h"

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
  text->highlight = false;
  text->padding_left = 0;
  text->padding_right = 0;

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
