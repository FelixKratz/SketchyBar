#ifndef TEXT_H
#define TEXT_H

#include <_types/_uint32_t.h>
#include <stdint.h>
struct text_line {
  CTLineRef line;
  CGFloat ascent;
  CGFloat descent;
  struct rgba_color color;
};

struct text {
  CGRect bounds;
  int y_offset;
  bool highlight;
  bool drawing;
  bool has_const_width;
  uint32_t custom_width;
  char* string;
  char* font_name;
  char align;
  CTFontRef font;
  int padding_left;
  int padding_right;
  struct text_line line;
  struct rgba_color color;
  struct rgba_color highlight_color;

  struct background background;
};

void text_init(struct text* text);
void text_clear_pointers(struct text* text);
void text_destroy_line(struct text* text);
void text_destroy(struct text* text);
bool text_set_string(struct text* text, char* string, bool forced);
bool text_set_color(struct text* text, uint32_t color);
bool text_set_font(struct text* text, char* font_string, bool forced);
uint32_t text_get_length(struct text* text, bool override);
uint32_t text_get_height(struct text* text);
bool text_update_color(struct text* text);

void text_calculate_bounds(struct text* text, uint32_t x, uint32_t y);
void text_draw(struct text* text, CGContextRef context);

static bool text_parse_sub_domain(struct text* text, FILE* rsp, struct token property, char* message);

#endif // !TEXT_H_
