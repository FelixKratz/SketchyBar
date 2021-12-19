#include "bar.h"
#include "alias.h"
#include "background.h"
#include "bar_item.h"
#include "display.h"
#include "graph.h"
#include "group.h"
#include "misc/helpers.h"
#include "text.h"
#include <_types/_uint32_t.h>
#include <stdint.h>

extern struct bar_manager g_bar_manager;


void bar_draw_graph(struct bar* bar, struct bar_item* bar_item, uint32_t x, bool right_to_left) {
  if (!bar_item->has_graph) return;
}

bool bar_draws_item(struct bar* bar, struct bar_item* bar_item) {
    if (!bar_item->drawing) return false;
    if (bar_item->associated_display > 0 && !(bar_item->associated_display & (1 << bar->adid))) return false;
    if (bar_item->associated_space > 0 && !(bar_item->associated_space & (1 << bar->sid)) && (bar_item->type != BAR_COMPONENT_SPACE)) return false;
    if (bar_item->position == POSITION_POPUP) return false;
    return true;
}

void bar_calculate_popup_anchor_for_bar_item(struct bar* bar, struct bar_item* bar_item) {
    bar_item->popup.cell_size = bar->frame.size.height;
    popup_calculate_bounds(&bar_item->popup);
    CGPoint anchor = bar->origin;
    anchor.x += bar_item->icon.bounds.origin.x - bar_item->background.padding_left;
    anchor.y += bar_item->icon.bounds.origin.y + (g_bar_manager.position == POSITION_BOTTOM ? (-bar->frame.size.height/2 - bar_item->popup.background.bounds.size.height) : bar->frame.size.height / 2);
    if (anchor.x + bar_item->popup.background.bounds.size.width > bar->frame.size.width) {
      anchor.x = bar->frame.size.width - bar_item->popup.background.bounds.size.width;
      popup_calculate_bounds(&bar_item->popup);
    }
    popup_set_anchor(&bar_item->popup, anchor, bar->adid);
    popup_calculate_bounds(&bar_item->popup);
}

void bar_draw_bar_items(struct bar* bar) {
  SLSDisableUpdate(g_connection);
  SLSOrderWindow(g_connection, bar->id, -1, 0);
  SLSRemoveAllTrackingAreas(g_connection, bar->id);

  draw_rect(bar->context, bar->frame, &g_bar_manager.background.color, g_bar_manager.background.corner_radius, g_bar_manager.background.border_width, &g_bar_manager.background.border_color, true);

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];

    if (!(bar_item->position == POSITION_POPUP)) bar_item_remove_associated_bar(bar_item, bar->adid);
    if (!bar_draws_item(bar, bar_item)) continue;

    bar_item_append_associated_bar(bar_item, bar->adid);

    if (bar_item->update_mask & UPDATE_MOUSE_ENTERED || bar_item->update_mask & UPDATE_MOUSE_EXITED)
      SLSAddTrackingRect(g_connection, bar->id, CGRectInset(bar_item_construct_bounding_rect(bar_item), 1, 1));

    bar_item_set_bounding_rect_for_display(bar_item, bar->adid, bar->origin, bar->frame.size.height);
    bar_item_draw(bar_item, bar->context);
  }

  CGContextFlush(bar->context);
  SLSOrderWindow(g_connection, bar->id, 1, bar->id);
  SLSReenableUpdate(g_connection);
}

void bar_redraw(struct bar* bar) {
  if (bar->hidden) return;
  if (bar->sid == 0) return;

  uint32_t bar_left_first_item_x = g_bar_manager.background.padding_left;
  uint32_t bar_right_first_item_x = bar->frame.size.width - g_bar_manager.background.padding_right;
  uint32_t bar_center_first_item_x = (bar->frame.size.width - 2*g_bar_manager.margin - bar_manager_length_for_bar_side(&g_bar_manager, bar, POSITION_CENTER)) / 2 - 1;
  uint32_t bar_center_right_first_item_x = (bar->frame.size.width + bar->notch_width) / 2 - 1;
  uint32_t bar_center_left_first_item_x = (bar->frame.size.width - bar->notch_width) / 2 - 1; 

  uint32_t* next_position = NULL;
  uint32_t y = bar->frame.size.height / 2;

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];

    if (!bar_draws_item(bar, bar_item)) continue;

    uint32_t bar_item_display_length = bar_item_get_length(bar_item, true);
    bool rtl = false;

    if (bar_item->position == POSITION_LEFT) next_position = &bar_left_first_item_x;
    else if (bar_item->position == POSITION_CENTER) next_position = &bar_center_first_item_x;
    else if (bar_item->position == POSITION_RIGHT) next_position = &bar_right_first_item_x, rtl = true;
    else if (bar_item->position == POSITION_CENTER_RIGHT) next_position = &bar_center_right_first_item_x;
    else if (bar_item->position == POSITION_CENTER_LEFT) next_position = &bar_center_left_first_item_x, rtl = true;
    else continue;

    if (bar_item->position == POSITION_RIGHT || bar_item->position == POSITION_CENTER_LEFT)
      *next_position -= bar_item_display_length + bar_item->background.padding_left + bar_item->background.padding_right;

    bar_item->graph.rtl = rtl;
    uint32_t bar_item_length = bar_item_calculate_bounds(bar_item, bar->frame.size.height - (g_bar_manager.background.border_width + 1), *next_position, y);

    if (bar_item->popup.drawing) bar_calculate_popup_anchor_for_bar_item(bar, bar_item);

    if (bar_item->position == POSITION_RIGHT || bar_item->position == POSITION_CENTER_LEFT) {
      *next_position += bar_item->has_const_width ? bar_item_display_length
                                                    + bar_item->background.padding_left
                                                    + bar_item->background.padding_right
                                                    - bar_item->custom_width : 0;
    } else 
      *next_position += bar_item_length + bar_item->background.padding_left + bar_item->background.padding_right;
  }
  bar_draw_bar_items(bar);
}

void bar_create_frame(struct bar *bar, CFTypeRef *frame_region) {
  CGRect bounds = display_bounds(bar->did);
  bounds.size.width -= 2*g_bar_manager.margin;
  CGPoint origin = bounds.origin;
  origin.x += g_bar_manager.margin;
  origin.y += g_bar_manager.y_offset;


  if (g_bar_manager.position == POSITION_BOTTOM) {
    origin.y = CGRectGetMaxY(bounds) - g_bar_manager.background.bounds.size.height - 2*g_bar_manager.y_offset;
  } else if (display_menu_bar_visible() && !g_bar_manager.topmost) {
    CGRect menu = display_menu_bar_rect(bar->did);
    origin.y += menu.size.height;
  }

  bar->frame = (CGRect) {{0, 0},{bounds.size.width, g_bar_manager.background.bounds.size.height}};
  bar->origin = origin;
  CGSNewRegionWithRect(&bar->frame, frame_region);
}

void bar_resize(struct bar *bar) {
  if (bar->hidden) return;
  CFTypeRef frame_region;
  bar_create_frame(bar, &frame_region);

  SLSDisableUpdate(g_connection);
  SLSOrderWindow(g_connection, bar->id, -1, 0);
  SLSSetWindowShape(g_connection, bar->id, bar->origin.x, bar->origin.y, frame_region);

  SLSClearActivationRegion(g_connection, bar->id);
  SLSAddActivationRegion(g_connection, bar->id, frame_region);
  SLSRemoveAllTrackingAreas(g_connection, bar->id);

  bar_redraw(bar);
  SLSOrderWindow(g_connection, bar->id, 1, 0);
  SLSReenableUpdate(g_connection);
  CFRelease(frame_region);
}

void bar_set_hidden(struct bar* bar, bool hidden) {
  if (bar->hidden == hidden) return;
  if (hidden) bar_close_window(bar);
  else bar_create_window(bar);
  bar->hidden = hidden;
}

void context_set_font_smoothing(CGContextRef context, bool smoothing) {
  CGContextSetAllowsFontSmoothing(context, smoothing);
}

void window_set_blur_radius(uint32_t wid) {
  SLSSetWindowBackgroundBlurRadius(g_connection, wid, g_bar_manager.blur_radius);
}

void window_disable_shadow(uint32_t wid) {
  CFIndex shadow_density = 0;
  CFNumberRef shadow_density_cf = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &shadow_density);
  const void *keys[1] = { CFSTR("com.apple.WindowShadowDensity") };
  const void *values[1] = { shadow_density_cf };
  CFDictionaryRef shadow_props_cf = CFDictionaryCreate(NULL, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  SLSWindowSetShadowProperties(wid, shadow_props_cf);
  CFRelease(shadow_density_cf);
  CFRelease(shadow_props_cf);
}

void bar_create_window(struct bar* bar) {
  uint64_t set_tags = kCGSStickyTagBit | kCGSHighQualityResamplingTagBit;
  uint64_t clear_tags = kCGSSuperStickyTagBit;

  CFTypeRef frame_region;
  bar_create_frame(bar, &frame_region);

  SLSNewWindow(g_connection, 2, bar->origin.x, bar->origin.y, frame_region, &bar->id);
  SLSAddActivationRegion(g_connection, bar->id, frame_region);
  CFRelease(frame_region);

  SLSSetWindowResolution(g_connection, bar->id, 2.0f);
  SLSSetWindowTags(g_connection, bar->id, &set_tags, 64);
  SLSClearWindowTags(g_connection, bar->id, &clear_tags, 64);
  SLSSetWindowOpacity(g_connection, bar->id, 0);
  window_set_blur_radius(bar->id);
  if (!g_bar_manager.shadow) window_disable_shadow(bar->id);

  SLSSetWindowLevel(g_connection, bar->id, g_bar_manager.window_level);
  bar->context = SLWindowContextCreate(g_connection, bar->id, 0);
  CGContextSetInterpolationQuality(bar->context, kCGInterpolationNone);
  context_set_font_smoothing(bar->context, g_bar_manager.font_smoothing);
}

void bar_close_window(struct bar* bar) {
  CGContextRelease(bar->context);
  SLSReleaseWindow(g_connection, bar->id);
}

struct bar *bar_create(uint32_t did) {
  struct bar *bar = malloc(sizeof(struct bar));
  memset(bar, 0, sizeof(struct bar));
  bar->hidden = false;
  bar->did = did;
  bar->sid = mission_control_index(display_space_id(did));
  bar->notch_width = CGDisplayIsBuiltin(did) ? g_bar_manager.notch_width : 0;
  bar_create_window(bar);
  return bar;
}

void bar_destroy(struct bar *bar) {
  bar_close_window(bar);
  free(bar);
}
