#pragma once
#include <stdbool.h>
#include "misc/helpers.h"
#include "window.h"
#include "image.h"

#define MENUBAR_LAYER 0x19

struct alias {
  bool permission;
  uint32_t update_frequency;
  uint32_t counter;

  char* name;
  char* owner;

  pid_t pid;

  struct window window;

  bool color_override;
  struct color color;
  struct image image;
};

void alias_init(struct alias* alias);
void alias_setup(struct alias* alias, char* owner, char* name);
uint32_t alias_get_length(struct alias* alias);
uint32_t alias_get_height(struct alias* alias);

void alias_calculate_bounds(struct alias* alias, uint32_t x, uint32_t y);
void alias_draw(struct alias* alias, CGContextRef context);
bool alias_update(struct alias* alias, bool forced);
void alias_destroy(struct alias* alias);

void print_all_menu_items(FILE* rsp);

bool alias_parse_sub_domain(struct alias* alias, FILE* rsp, struct token property, char* message);
