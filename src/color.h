#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "misc/helpers.h"

struct color {
  float r;
  float g;
  float b;
  float a;
  uint32_t hex;
};

static struct color g_transparent = { 0 };

void color_init(struct color* color, uint32_t hex);
bool color_set_hex(struct color* color, uint32_t hex);
bool color_set_alpha(struct color* color, float alpha);
bool color_set_r(struct color* color, float red);
bool color_set_g(struct color* color, float green);
bool color_set_b(struct color* color, float blue);

bool color_parse_sub_domain(struct color* color, FILE* rsp, struct token property, char* message);
