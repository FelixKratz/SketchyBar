#pragma once
#include "misc/helpers.h"

#define kCGSExposeFadeTagBit         (1ULL <<  1)
#define kCGSPreventsActivationTagBit (1ULL <<  16)

#define W_ABOVE  1
#define W_OUT    0
#define W_BELOW  -1

extern CFTypeRef g_transaction;

struct window {
  struct window* parent;
  int order_mode;
  bool needs_move;
  bool needs_resize;

  uint32_t id;
  uint32_t surface_id;

  CGRect frame;
  CGPoint origin;
  CGContextRef context;
};

void window_init(struct window* window);
void window_create(struct window* window, CGRect frame);
void window_close(struct window* window);
void window_clear(struct window* window);
void window_flush(struct window* window);

void window_move(struct window* window, CGPoint point);
void window_set_frame(struct window* window, CGRect frame);
bool window_apply_frame(struct window* window, bool forced);
void window_send_to_space(struct window* window, uint64_t dsid);

void window_set_blur_radius(struct window* window, uint32_t blur_radius);
void window_disable_shadow(struct window* window);
void window_set_level(struct window* window, uint32_t level);
void window_order(struct window* window, struct window* parent, int mode);
void window_assign_mouse_tracking_area(struct window* window, CGRect rect);

CGImageRef window_capture(struct window* window, bool* disabled);

void context_set_font_smoothing(CGContextRef context, bool smoothing);

void windows_freeze();
void windows_unfreeze();
