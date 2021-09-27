#include "text.h"
#include "misc/helpers.h"

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

bool text_set_string(struct text* text, char* string, bool forced) {
  if (!string) return false;
  if (!forced && text->string && strcmp(text->string, string) == 0) { 
    if (!(string == text->string)) free(string);
    return false; 
  }
  if (text->line.line) bar_destroy_line(&text->line);
  if (string != text->string && !text->string) free(text->string);
  text->string = string;
  bar_prepare_line(&text->line, text->font, text->string, text->highlight ? text->highlight_color : text->color);
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

  text->font = bar_create_font(font_string);
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

void text_destroy(struct text* text) {
  if (text->string) free(text->string);
  if (text->font_name) free(text->font_name);
  if (text->font) CFRelease(text->font);
  if (text->line.line) CFRelease(text->line.line);
  text_clear_pointers(text);
}
