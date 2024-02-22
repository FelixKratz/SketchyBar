#include "font.h"
#include "animation.h"
#include "bar_manager.h"

void font_register(char* font_path) {
  CFStringRef url_string = CFStringCreateWithCString(kCFAllocatorDefault,
                                                     font_path,
                                                     kCFStringEncodingUTF8);
  if (url_string) {
    CFURLRef url_ref = CFURLCreateWithString(kCFAllocatorDefault,
                                             url_string,
                                             NULL                );
    if (url_ref) {
      CTFontManagerRegisterFontsForURL(url_ref,
                                       kCTFontManagerScopeProcess,
                                       NULL                       );
      CFRelease(url_ref);
    }
    CFRelease(url_string);
  }
  free(font_path);
}

void font_create_ctfont(struct font* font) {

  CFStringRef family_ref = CFStringCreateWithCString(NULL,
                                                     font->family,
                                                     kCFStringEncodingUTF8);

  CFStringRef style_ref = CFStringCreateWithCString(NULL,
                                                    font->style,
                                                    kCFStringEncodingUTF8);

  CFNumberRef size_ref = CFNumberCreate(NULL,
                                        kCFNumberFloat32Type,
                                        &font->size          );

  const void *keys[] = { kCTFontFamilyNameAttribute,
                         kCTFontStyleNameAttribute,
                         kCTFontSizeAttribute       };

  const void *values[] = { family_ref, style_ref, size_ref };
  CFDictionaryRef attr = CFDictionaryCreate(NULL,
                                            keys,
                                            values,
                                            array_count(keys),
                                            &kCFTypeDictionaryKeyCallBacks,
                                            &kCFTypeDictionaryValueCallBacks);

  CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attr);

  if (font->ct_font) CFRelease(font->ct_font);
  font->ct_font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);

  CFRelease(descriptor);
  CFRelease(attr);
  CFRelease(size_ref);
  CFRelease(style_ref);
  CFRelease(family_ref);
}

void font_init(struct font* font) {
  font->size = 14.f;
  font->style = string_copy("Bold");
  font->family = string_copy("Hack Nerd Font");
  font_create_ctfont(font);
}

bool font_set_style(struct font* font, char* style, bool forced) {
  if (!style) return false;
  if (!forced && font->style && string_equals(font->style, style)) {
    free(style);
    return false;
  }
  if (font->style && style != font->style) free(font->style);
  font->style = style;
  font->font_changed = true;

  return true;
}

bool font_set_family(struct font* font, char* family, bool forced) {
  if (!family) return false;
  if (!forced && font->family && string_equals(font->family, family)) {
    free(family);
    return false;
  }
  if (font->family) free(font->family);
  font->family = family;
  font->font_changed = true;

  return true;
}

bool font_set_size(struct font* font, float size) {
  if (font->size == size) return false;

  font->size = size;
  font->font_changed = true;

  return true;
}

bool font_set(struct font* font, char* font_string, bool forced) {
  if (!font_string) return false;

  float size = 10.0f;
  char font_properties[2][255] = { {}, {} };
  sscanf(font_string,
         "%254[^:]:%254[^:]:%f",
         font_properties[0],
         font_properties[1],
         &size                  );

  free(font_string);

  bool change = font_set_family(font, string_copy(font_properties[0]), forced);
  change |= font_set_style(font, string_copy(font_properties[1]), forced);
  change |= font_set_size(font, size);

  return change;
}

void font_clear_pointers(struct font* font) {
  font->ct_font = NULL;
  font->family = NULL;
  font->style = NULL;
}

void font_destroy(struct font* font) {
  if (font->style) free(font->style);
  if (font->family) free(font->family);
  if (font->ct_font) CFRelease(font->ct_font);
  font_clear_pointers(font);
}

bool font_parse_sub_domain(struct font* font, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_FONT_SIZE)) {
    struct token token = get_token(&message);
    ANIMATE_FLOAT(font_set_size,
                  font,
                  font->size,
                  token_to_float(token));
  } else if (token_equals(property, PROPERTY_FONT_FAMILY)) {
    struct token token = get_token(&message);
    needs_refresh = font_set_family(font, token_to_string(token), false);
  } else if (token_equals(property, PROPERTY_FONT_STYLE)) {
    struct token token = get_token(&message);
    needs_refresh = font_set_style(font, token_to_string(token), false);
  } else {
    respond(rsp, "[!] Text: Invalid property '%s'\n", property.text);
  }

  return needs_refresh;
}
