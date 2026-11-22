#include "tooltip.h"
#include "bar_item.h"
#include "bar_manager.h"

#define TOOLTIP_PADDING 8

void tooltip_init(struct tooltip* tooltip) {
  tooltip->host = NULL;
  tooltip->anchor = CGRectNull;

  text_init(&tooltip->text);
  tooltip->text.padding_left = TOOLTIP_PADDING;
  tooltip->text.padding_right = TOOLTIP_PADDING;
  tooltip->text.background.enabled = true;
  tooltip->text.background.corner_radius = 5;
  color_set_hex(&tooltip->text.background.color, 0xf0202020);

  window_init(&tooltip->window);
}

void tooltip_hide(struct tooltip* tooltip) {
  tooltip->host = NULL;
  window_close(&tooltip->window);
}

void tooltip_show(struct tooltip* tooltip, struct bar_item* host, CGRect anchor) {
  tooltip_hide(tooltip);
  if (!host->tooltip || strlen(host->tooltip) == 0) return;

  text_set_string(&tooltip->text, string_copy(host->tooltip), false);
  uint32_t width = text_get_length(&tooltip->text, false);
  uint32_t height = text_get_height(&tooltip->text) + TOOLTIP_PADDING;
  background_set_height(&tooltip->text.background, height);

  CGPoint origin = anchor.origin;
  origin.x += ((int)anchor.size.width - (int)width) / 2;
  origin.y += (g_bar_manager.position == POSITION_BOTTOM
               ? -(int)height
               : anchor.size.height);

  uint32_t did = 0, count = 0;
  CGGetDisplaysWithPoint(anchor.origin, 1, &did, &count);
  if (count > 0) {
    CGRect bounds = CGDisplayBounds(did);
    if (origin.x + width > CGRectGetMaxX(bounds))
      origin.x = CGRectGetMaxX(bounds) - width;
    if (origin.x < bounds.origin.x) origin.x = bounds.origin.x;
  }

  window_open(&tooltip->window, (CGRect){origin, {width, height}});
  window_disable_shadow(&tooltip->window);
  window_set_level(&tooltip->window, kCGPopUpMenuWindowLevel);
  window_order(&tooltip->window, NULL, W_ABOVE);

  CGContextRef context = tooltip->window.surface->context;
  CGContextClearRect(context, tooltip->window.frame);
  text_calculate_bounds(&tooltip->text, 0, height / 2);
  text_draw(&tooltip->text, context);
  CGContextFlush(context);
  window_flush(&tooltip->window);

  tooltip->host = host;
  tooltip->anchor = anchor;
}
