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

  // Ownership and deferred updates are serialized on the main queue.
  int32_t refc;
  CGRect frame;
};

struct surface* surface_create(struct window* window);
void surface_destroy(struct surface* surface);
void surface_retain(struct surface* surface);
void surface_release(struct surface* surface);

void surface_resize(struct surface* surface, struct window* window);
void surface_flush(struct surface* surface);

