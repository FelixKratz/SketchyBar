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
    return true;
}

void bar_draw_bar_items(struct bar* bar) {
  SLSDisableUpdate(g_connection);
  SLSOrderWindow(g_connection, bar->id, -1, 0);
  SLSRemoveAllTrackingAreas(g_connection, bar->id);

  draw_rect(bar->context, bar->frame, &g_bar_manager.background.color, g_bar_manager.background.corner_radius, g_bar_manager.background.border_width, &g_bar_manager.background.border_color, true);

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];

    bar_item_remove_associated_bar(bar_item, bar->adid);
    if (!bar_draws_item(bar, bar_item)) continue;

    bar_item_append_associated_bar(bar_item, bar->adid);

    if (bar_item->update_mask & UPDATE_MOUSE_ENTERED || bar_item->update_mask & UPDATE_MOUSE_EXITED)
      SLSAddTrackingRect(g_connection, bar->id, CGRectInset(bar_item_construct_bounding_rect(bar_item), 1, 1));

    bar_item_set_bounding_rect_for_display(bar_item, bar->adid, bar->origin);

    if (bar_item->group && group_is_first_member(bar_item->group, bar_item))
      group_draw(bar_item->group, bar->context);

    background_draw(&bar_item->background, bar->context);
    text_draw(&bar_item->icon, bar->context);
    text_draw(&bar_item->label, bar->context);

    if (bar_item->has_alias)
      alias_draw(&bar_item->alias, bar->context);
    if (bar_item->has_graph)
      graph_draw(&bar_item->graph, bar->context);
  }

  CGContextFlush(bar->context);
  SLSOrderWindow(g_connection, bar->id, 1, bar->id);
  SLSReenableUpdate(g_connection);
}

void bar_redraw(struct bar* bar) {
  if (bar->hidden) return;
  if (bar->sid == 0) return;

  uint32_t bar_left_first_item_x = g_bar_manager.background.padding_left;
  uint32_t bar_right_first_item_x = bar->frame.size.width - g_bar_manager.background.padding_right + 1;
  uint32_t bar_center_first_item_x = (bar->frame.size.width - bar_manager_length_for_bar_side(&g_bar_manager, bar, POSITION_CENTER)) / 2;

  uint32_t* next_position = NULL;

  uint32_t y = bar->frame.size.height / 2;

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];

    if (!bar_draws_item(bar, bar_item)) continue;

    uint32_t bar_item_length = bar_item_get_length(bar_item, false);
    uint32_t bar_item_display_length = bar_item_get_length(bar_item, true);

    uint32_t icon_position = 0;
    uint32_t label_position = 0;
    uint32_t sandwich_position = 0;
    bool rtl = false;

    if (bar_item->position == POSITION_LEFT) next_position = &bar_left_first_item_x;
    else if (bar_item->position == POSITION_CENTER) next_position = &bar_center_first_item_x;
    else {
      next_position = &bar_right_first_item_x;
      rtl = true;
    }

    if (bar_item->position == POSITION_RIGHT)
      *next_position -= bar_item_display_length + bar_item->background.padding_left + bar_item->background.padding_right;

    icon_position = *next_position + bar_item->background.padding_left;
    label_position = icon_position + text_get_length(&bar_item->icon, false);

    sandwich_position = label_position;
    if (bar_item->has_graph) {
      label_position += graph_get_length(&bar_item->graph);
    } else if (bar_item->has_alias) {
      label_position += alias_get_length(&bar_item->alias); 
    }

    if (bar_item->group && group_is_first_member(bar_item->group, bar_item))
      group_calculate_bounds(bar_item->group, bar_item->position == POSITION_RIGHT ? 
          (icon_position - group_get_length(bar_item->group) + bar_item->group->num_members - 1 + bar_item_length) : 
          icon_position, y);

    text_calculate_bounds(&bar_item->icon, icon_position, y + bar_item->y_offset);
    text_calculate_bounds(&bar_item->label, label_position, y + bar_item->y_offset);

    if (bar_item->has_alias)
      alias_calculate_bounds(&bar_item->alias, sandwich_position, y + bar_item->y_offset);

    if (bar_item->has_graph) {
      bar_item->graph.bounds.size.height = bar_item->background.enabled ? (bar_item->background.bounds.size.height - bar_item->background.border_width - 1)
                                                                      : (bar->frame.size.height - (g_bar_manager.background.border_width + 1));
      bar_item->graph.rtl = rtl;
      graph_calculate_bounds(&bar_item->graph, sandwich_position, y + bar_item->y_offset);
    }

    bar_item->background.bounds.size.height = bar_item->background.overrides_height ? bar_item->background.bounds.size.height
                                                                                    : (bar->frame.size.height - (g_bar_manager.background.border_width + 1));
    bar_item->background.bounds.size.width = bar_item_length;
    background_calculate_bounds(&bar_item->background, icon_position, y + bar_item->y_offset);

    if (bar_item->position == POSITION_RIGHT) {
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

void bar_set_font_smoothing(struct bar* bar, bool smoothing) {
  CGContextSetAllowsFontSmoothing(bar->context, smoothing);
}

void bar_set_blur_radius(struct bar* bar) {
  SLSSetWindowBackgroundBlurRadius(g_connection, bar->id, g_bar_manager.blur_radius);
}

void bar_disable_shadow(struct bar* bar) {
  CFIndex shadow_density = 0;
  CFNumberRef shadow_density_cf = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &shadow_density);
  const void *keys[1] = { CFSTR("com.apple.WindowShadowDensity") };
  const void *values[1] = { shadow_density_cf };
  CFDictionaryRef shadow_props_cf = CFDictionaryCreate(NULL, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  SLSWindowSetShadowProperties(bar->id, shadow_props_cf);
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
  bar_set_blur_radius(bar);
  if (!g_bar_manager.shadow) bar_disable_shadow(bar);

  SLSSetWindowLevel(g_connection, bar->id, g_bar_manager.window_level);
  bar->context = SLWindowContextCreate(g_connection, bar->id, 0);
  CGContextSetInterpolationQuality(bar->context, kCGInterpolationNone);
  bar_set_font_smoothing(bar, g_bar_manager.font_smoothing);
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
  bar_create_window(bar);
  return bar;
}

void bar_destroy(struct bar *bar) {
  bar_close_window(bar);
  free(bar);
}
