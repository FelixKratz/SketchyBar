#ifndef TEXT_H
#define TEXT_H

struct text {
  bool highlight;
  struct bar_line line;
  char* string;
  char* font_name;
  CTFontRef font;
  int padding_left;
  int padding_right;
  struct rgba_color color;
  struct rgba_color highlight_color;
};

void text_init(struct text* text);
void text_clear_pointers(struct text* text);
void text_destroy(struct text* text);
bool text_set_string(struct text* text, char* string, bool forced);
bool text_set_color(struct text* text, uint32_t color);
bool text_set_font(struct text* text, char* font_string, bool forced);
bool text_update_color(struct text* text);

#endif // !TEXT_H_
