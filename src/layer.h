#pragma once
#include <CoreGraphics/CoreGraphics.h>

typedef void* CALayerRef;
typedef void* CAContextRef;

struct layer {
  CAContextRef context;
  CALayerRef root;
};

struct layer* layer_create(uint32_t cid, CGRect bounds);
void layer_destroy(struct layer* layer);

uint32_t layer_get_context_id(struct layer* layer);
void layer_set_bounds(struct layer* layer, CGRect bounds);
void layer_set_contents(struct layer* layer, CGImageRef image);
