#pragma once
#include <CoreText/CoreText.h>
#include "misc/helpers.h"

struct font {
  CTFontRef ct_font;

  bool font_changed;
  float size;
  char* family;
  char* style;
};

void font_register(char* font_path);

void font_init(struct font* font);
void font_destroy(struct font* font);
bool font_set(struct font* font, char* font_string, bool forced);
bool font_set_size(struct font* font, float size);
bool font_set_family(struct font* font, char* family, bool forced);
bool font_set_style(struct font* font, char* style, bool forced);
void font_create_ctfont(struct font* font);
void font_clear_pointers(struct font* font);

bool font_parse_sub_domain(struct font* font, FILE* rsp, struct token property, char* message);
