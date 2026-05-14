#pragma once
#include <CoreText/CoreText.h>
#include "background.h"
#include "font.h"

struct text_line {
  CTLineRef line;
  CGFloat ascent;
  CGFloat descent;
};

enum badge_anchor {
  BADGE_ANCHOR_TOP_LEFT,
  BADGE_ANCHOR_TOP_CENTER,
  BADGE_ANCHOR_TOP_RIGHT,
  BADGE_ANCHOR_CENTER_LEFT,
  BADGE_ANCHOR_CENTER,
  BADGE_ANCHOR_CENTER_RIGHT,
  BADGE_ANCHOR_BOTTOM_LEFT,
  BADGE_ANCHOR_BOTTOM_CENTER,
  BADGE_ANCHOR_BOTTOM_RIGHT,
};

struct badge {
  bool drawing;
  bool has_const_width;

  char align;
  char* string;

  int x_offset;
  int y_offset;
  int text_x_offset;
  int text_y_offset;
  uint32_t custom_width;
  float width;

  CGRect bounds;

  enum badge_anchor anchor;
  struct font font;
  struct text_line line;
  struct color color;
  struct background background;
};

struct text {
  bool highlight;
  bool drawing;
  bool has_const_width;

  char align;
  char* string;

  int x_offset;
  int y_offset;
  int padding_left;
  int padding_right;
  uint32_t custom_width;
  uint32_t max_chars;
  uint32_t scroll_duration;
  float scroll;
  float width;

  CGRect bounds;

  struct font font;
  struct text_line line;
  struct color color;
  struct color highlight_color;
  struct shadow shadow;

  struct background background;
  struct badge badge;
};

void text_init(struct text* text);
void text_clear_pointers(struct text* text);
uint32_t text_get_length(struct text* text, bool override);
uint32_t text_get_height(struct text* text);
bool text_set_string(struct text* text, char* string, bool forced);
bool text_set_font(struct text* text, char* font_string, bool forced);
void text_copy(struct text* text, struct text* source);

bool text_animate_scroll(struct text* text);
void text_calculate_bounds(struct text* text, uint32_t x, uint32_t y);
void text_draw(struct text* text, CGContextRef context);
void text_draw_badge(struct text* text, CGContextRef context);
void text_destroy(struct text* text);

void text_serialize(struct text* text, char* indent, FILE* rsp);
bool text_parse_sub_domain(struct text* text, FILE* rsp, struct token property, char* message);
