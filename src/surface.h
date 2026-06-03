#pragma once
#include <CoreGraphics/CoreGraphics.h>
#include "context.h"

struct window;
struct layer;

struct surface {
  struct layer* layer;
  CGContextRef context;

  uint32_t id;
  uint32_t wid;
};

struct surface* surface_create(struct window* window);
void surface_destroy(struct surface* surface);

void surface_resize(struct surface* surface, struct window* window);
void surface_flush(struct surface* surface);

