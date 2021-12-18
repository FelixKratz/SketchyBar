#include "popup.h"
#include "background.h"
#include "bar_item.h"
#include "misc/helpers.h"
#include <stdint.h>

void popup_init(struct popup* popup) {
  popup->drawing = false;
  popup->horizontal = false;
  popup->context = NULL;
  popup->id = 0;
  popup->frame.origin = (CGPoint){0,0};
  popup->anchor = (CGPoint){100, 100};
  
  popup->num_items = 0;
  popup->cell_size = 30;
  popup->items = NULL;
  background_init(&popup->background);
  popup->background.color = rgba_color_from_hex(0xff000000);
}

void popup_calculate_bounds(struct popup* popup) {
  uint32_t y = popup->cell_size / 2;
  uint32_t width = 0;
  for (int i = 0; i < popup->num_items; i++) {
    uint32_t item_width = popup->items[i]->background.padding_right + popup->items[i]->background.padding_left + bar_item_calculate_bounds(popup->items[i], popup->cell_size, 0, y);
    if (item_width > width) width = item_width;
    y += popup->cell_size;
  }
  popup->background.bounds.size.width = width;
  popup->background.bounds.size.height = y - popup->cell_size / 2;
}

void popup_create_frame(struct popup *popup, CFTypeRef *frame_region) {
  popup->frame.size = popup->background.bounds.size;
  CGSNewRegionWithRect(&popup->frame, frame_region);
}

void popup_create_window(struct popup* popup) {
  uint64_t set_tags = kCGSStickyTagBit | kCGSHighQualityResamplingTagBit;
  uint64_t clear_tags = kCGSSuperStickyTagBit;

  CFTypeRef frame_region;
  popup_calculate_bounds(popup);
  popup_create_frame(popup, &frame_region);

  SLSNewWindow(g_connection, 2, popup->anchor.x, popup->anchor.y, frame_region, &popup->id);
  SLSAddActivationRegion(g_connection, popup->id, frame_region);
  CFRelease(frame_region);

  SLSSetWindowResolution(g_connection, popup->id, 2.0f);
  SLSSetWindowTags(g_connection, popup->id, &set_tags, 64);
  SLSClearWindowTags(g_connection, popup->id, &clear_tags, 64);
  SLSSetWindowOpacity(g_connection, popup->id, 0);
  SLSSetWindowBackgroundBlurRadius(g_connection, popup->id, g_bar_manager.blur_radius);
  window_disable_shadow(popup->id);

  SLSSetWindowLevel(g_connection, popup->id, NSScreenSaverWindowLevel);
  popup->context = SLWindowContextCreate(g_connection, popup->id, 0);
  CGContextSetInterpolationQuality(popup->context, kCGInterpolationNone);
  context_set_font_smoothing(popup->context, g_bar_manager.font_smoothing);

  popup->drawing = true;
  popup_draw(popup);
}

void popup_close_window(struct popup* popup) {
  CGContextRelease(popup->context);
  SLSReleaseWindow(g_connection, popup->id);

  popup->context = NULL;
  popup->id = false;
  popup->drawing = false;
}

void popup_add_item(struct popup* popup, struct bar_item* bar_item) {
  popup->num_items++;
  popup->items = realloc(popup->items, sizeof(struct bar_item*)*popup->num_items);
  popup->items[popup->num_items - 1] = bar_item;
}

void popup_set_anchor(struct popup* popup, CGPoint anchor) {
  popup->anchor = anchor;

  if (popup->drawing) {
    //popup_close_window(popup);
    //popup_create_window(popup);
  }
}

void popup_set_drawing(struct popup* popup, bool drawing) {
  if (!drawing && popup->drawing) popup_close_window(popup);
  else if (drawing && !popup->drawing) popup_create_window(popup);
}

void popup_draw(struct popup* popup) {
  if (!popup->drawing) return;
  popup_calculate_bounds(popup);

  SLSDisableUpdate(g_connection);
  SLSOrderWindow(g_connection, popup->id, -1, 0);
  draw_rect(popup->context, popup->frame, &popup->background.color, popup->background.corner_radius, popup->background.border_width, &popup->background.border_color, true);

  for (int i = 0; i < popup->num_items; i++) {
    bool state = popup->items[i]->popup.drawing;
    popup->items[i]->popup.drawing = false;
    bar_item_draw(popup->items[i], popup->context);
    popup->items[i]->popup.drawing = state;
  }
  CGContextFlush(popup->context);
  SLSOrderWindow(g_connection, popup->id, 1, popup->id);
  SLSReenableUpdate(g_connection);
}

void popup_destroy(struct popup* popup) {
  for (int i = 0; i < popup->num_items; i++) {
    free(popup->items[i]);
  }
  if (popup->items) free(popup->items);
  if (popup->context) free(popup->context);
}

