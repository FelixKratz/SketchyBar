#include "bar.h"
#include "alias.h"
#include "bar_item.h"
#include "display.h"
#include "misc/helpers.h"
#include <_types/_uint32_t.h>
#include <stdint.h>

extern struct bar_manager g_bar_manager;

static CTFontRef bar_create_font(char *cstring) {
  float size = 10.0f;
  char font_properties[2][255] = { {}, {} };
  sscanf(cstring, "%254[^:]:%254[^:]:%f", font_properties[0], font_properties[1], &size);
  CFStringRef font_family_name = CFStringCreateWithCString(NULL, font_properties[0], kCFStringEncodingUTF8);
  CFStringRef font_style_name = CFStringCreateWithCString(NULL, font_properties[1], kCFStringEncodingUTF8);
  CFNumberRef font_size = CFNumberCreate(NULL, kCFNumberFloat32Type, &size);

  const void *keys[] = { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute, kCTFontSizeAttribute };
  const void *values[] = { font_family_name, font_style_name, font_size };
  CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
  CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);

  CFRelease(descriptor);
  CFRelease(attributes);
  CFRelease(font_size);
  CFRelease(font_style_name);
  CFRelease(font_family_name);

  return font;
}

static CGPoint bar_align_line(struct bar *bar, struct bar_line* line, int align_x, int align_y) {
  float x = 0, y = 0;

  if (align_x == ALIGN_NONE) {
    x = CGContextGetTextPosition(bar->context).x;
  } else if (align_x == ALIGN_LEFT) {
    x = 20;
  } else if (align_x == ALIGN_CENTER) {
    x = (bar->frame.size.width / 2) - (line->bounds.size.width  / 2);
  } else if (align_x == ALIGN_RIGHT) {
    x = bar->frame.size.width - line->bounds.size.width - 20;
  }

  if (align_y == ALIGN_NONE) {
    y = CGContextGetTextPosition(bar->context).y;
  } else if (align_y == ALIGN_TOP) {
    y = bar->frame.size.height;
  } else if (align_y == ALIGN_CENTER) {
    y = (bar->frame.size.height / 2) - ((line->ascent - line->descent) / 2);
  } else if (align_y == ALIGN_BOTTOM) {
    y = line->descent;
  }

  return (CGPoint) { x, y };
}

static void bar_draw_line(struct bar *bar, struct bar_line* line, float x, float y) {
  CGContextSetRGBFillColor(bar->context, line->color.r, line->color.g, line->color.b, line->color.a);
  CGContextSetTextPosition(bar->context, x, y);
  CTLineDraw(line->line, bar->context);
}

static void bar_destroy_line(struct bar_line* line) {
  CFRelease(line->line);
}

void bar_prepare_line(struct bar_line* bar_line, CTFontRef font, char* cstring, struct rgba_color color) {
  const void *keys[] = { kCTFontAttributeName, kCTForegroundColorFromContextAttributeName };
  const void *values[] = { font, kCFBooleanTrue };
  CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFStringRef string = CFStringCreateWithCString(NULL, cstring, kCFStringEncodingUTF8);
  if (!string) string = CFStringCreateWithCString(NULL, "Warning: Malformed UTF-8 string", kCFStringEncodingUTF8);
  CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL, string, attributes);
  bar_line->line = CTLineCreateWithAttributedString(attr_string);

  CTLineGetTypographicBounds(bar_line->line, &bar_line->ascent, &bar_line->descent, NULL);
  bar_line->bounds = CTLineGetBoundsWithOptions(bar_line->line, kCTLineBoundsUseGlyphPathBounds);
  bar_line->color = color;

  CFRelease(string);
  CFRelease(attributes);
  CFRelease(attr_string);
}

void bar_draw_graph_line(struct bar *bar, struct graph_data* graph_data, uint32_t x, uint32_t y, bool right_to_left) {
  const float height = bar->frame.size.height * 0.9f;
  uint32_t sample_width = 1;
  bool fill = graph_data->fill;
  CGContextSaveGState(bar->context);
  CGContextSetRGBStrokeColor(bar->context, graph_data->line_color.r, graph_data->line_color.g, graph_data->line_color.b, graph_data->line_color.a);
  if (graph_data->overrides_fill_color) CGContextSetRGBFillColor(bar->context, graph_data->fill_color.r, graph_data->fill_color.g, graph_data->fill_color.b, graph_data->fill_color.a);
  else CGContextSetRGBFillColor(bar->context, graph_data->line_color.r, graph_data->line_color.g, graph_data->line_color.b, 0.2 * graph_data->line_color.a);
  CGContextSetLineWidth(bar->context, graph_data->line_width);
  CGMutablePathRef p = CGPathCreateMutable();
  uint32_t start_x = x;
  if (right_to_left) {
    CGPathMoveToPoint(p, NULL, x, y + graph_data_get_y(graph_data, graph_data->graph_width - 1) * height);
    for (int i = graph_data->graph_width - 1; i > 0; --i, x -= sample_width) {
      CGPathAddLineToPoint(p, NULL, x, y + graph_data_get_y(graph_data, i) * height);
    }
  }
  else {
    CGPathMoveToPoint(p, NULL, x, y + graph_data_get_y(graph_data, 0) * height);
    for (int i = graph_data->graph_width - 1; i > 0; --i, x += sample_width) {
      CGPathAddLineToPoint(p, NULL, x, y + graph_data_get_y(graph_data, i) * height);
    }
  }
  CGContextAddPath(bar->context, p);
  CGContextStrokePath(bar->context);
  if (fill) {
    if (right_to_left) {
      CGPathAddLineToPoint(p, NULL, x + sample_width, y);
      CGPathAddLineToPoint(p, NULL, start_x, y);
    }
    else {
      CGPathAddLineToPoint(p, NULL, x - sample_width, y);
      CGPathAddLineToPoint(p, NULL, start_x, y);
    }
    CGPathCloseSubpath(p);
    CGContextAddPath(bar->context, p);
    CGContextFillPath(bar->context);
  }
  CGPathRelease(p);
  CGContextRestoreGState(bar->context);
}

static int bar_get_center_length(struct bar_manager* bar_manager) {
  int total_length = 0;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->position == BAR_POSITION_CENTER) {
      total_length += bar_item->label_line.bounds.size.width + bar_item->icon_line.bounds.size.width + bar_item->icon_spacing_right + bar_item->label_spacing_left + (bar_item->has_graph ? bar_item->graph_data.graph_width : 0);
      if (i > 0) {
        total_length += bar_manager->bar_items[i-1]->label_spacing_right + bar_item->icon_spacing_left;
      }
    }
  }
  return total_length;
}

void bar_draw_graph(struct bar* bar, struct bar_item* bar_item, uint32_t x, bool right_to_left) {
  if (!bar_item->has_graph) return;
  bar_draw_graph_line(bar, &bar_item->graph_data, x, g_bar_manager.border_width + 1, right_to_left);
}

void bar_draw_item_background(struct bar* bar, struct bar_item* bar_item, uint32_t sid) {
  if (!bar_item->draws_background) return;
  bool custom_height = bar_item->background_height != 0;
  CGRect draw_region = {{bar_item->bounding_rects[sid - 1]->origin.x - bar->origin.x, custom_height ? ((bar->frame.size.height - bar_item->background_height)) / 2 : (g_bar_manager.border_width + 1)},
                        {bar_item->bounding_rects[sid - 1]->size.width, custom_height ? bar_item->background_height : (bar->frame.size.height - 2*(g_bar_manager.border_width + 1))}};
  draw_region = CGRectInset(draw_region, bar_item->background_border_width / 2, bar_item->background_border_width / 2);
  draw_rect(bar->context, draw_region, &bar_item->background_color, bar_item->background_corner_radius, bar_item->background_border_width, &bar_item->background_border_color, false);
}

void bar_draw_alias(struct bar* bar, struct bar_item* bar_item, uint32_t x) {
  if (!bar_item->has_alias || !bar_item->alias.image_ref) return;
  CGRect bounds = {{x, (bar->frame.size.height - bar_item->alias.bounds.size.height) / 2},{bar_item->alias.bounds.size.width, bar_item->alias.bounds.size.height}};
  CGContextDrawImage(bar->context, bounds, bar_item->alias.image_ref);
}

void bar_redraw(struct bar* bar) {
  if (bar->hidden) return;
  uint32_t did = bar->adid;
  uint32_t sid = bar->sid;
  if (sid == 0) return;

  int bar_left_final_item_x = g_bar_manager.padding_left;
  int bar_right_first_item_x = bar->frame.size.width - g_bar_manager.padding_right;
  int bar_center_first_item_x = (bar->frame.size.width - bar_get_center_length(&g_bar_manager)) / 2;

  SLSDisableUpdate(g_connection);
  SLSOrderWindow(g_connection, bar->id, -1, 0);
  draw_rect(bar->context, bar->frame, &g_bar_manager.background_color, g_bar_manager.corner_radius, g_bar_manager.border_width, &g_bar_manager.border_color, true);

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];

    if (!bar_item->drawing) continue;
    if (bar_item->associated_display > 0 && !(bar_item->associated_display & (1 << did))) continue;
    if (bar_item->associated_space > 0 && !(bar_item->associated_space & (1 << sid)) && (bar_item->type != BAR_COMPONENT_SPACE)) continue;

    struct bar_line* label = &bar_item->label_line;
    struct bar_line* icon = &bar_item->icon_line;
    CGPoint icon_position = bar_align_line(bar, icon, ALIGN_CENTER, ALIGN_CENTER);
    CGPoint label_position = bar_align_line(bar, label, ALIGN_CENTER, ALIGN_CENTER);
    uint32_t graph_x = 0;
    bool graph_rtl = false;

    if (bar_item->position == BAR_POSITION_LEFT) {
      icon_position.x = bar_left_final_item_x + bar_item->icon_spacing_left;
      label_position.x = icon_position.x + icon->bounds.size.width + bar_item->icon_spacing_right + bar_item->label_spacing_left;
      
      if (!bar_item->nospace)
        bar_left_final_item_x = label_position.x + label->bounds.size.width + bar_item->label_spacing_right;
      if (bar_item->has_graph) {
        graph_x = bar_item->nospace ? label_position.x + label->bounds.size.width + bar_item->label_spacing_right : bar_left_final_item_x;
        if (!bar_item->nospace)
          bar_left_final_item_x += bar_item->graph_data.graph_width;
      }
      if (bar_item->has_alias) {
        bar_left_final_item_x += bar_item->alias.size.x;
      }
    }
    else if (bar_item->position == BAR_POSITION_RIGHT) {
      label_position.x = bar_right_first_item_x - label->bounds.size.width - bar_item->label_spacing_right;
      icon_position.x = label_position.x - icon->bounds.size.width - bar_item->icon_spacing_right - bar_item->label_spacing_left + 1;

      if (!bar_item->nospace)
        bar_right_first_item_x = icon_position.x - bar_item->icon_spacing_left;
      if (bar_item->has_graph) {
        graph_x = bar_item->nospace ? icon_position.x - bar_item->icon_spacing_left : bar_right_first_item_x;
        graph_rtl = true;
        if (!bar_item->nospace)
          bar_right_first_item_x -= bar_item->graph_data.graph_width;
      }
      if (bar_item->has_alias) {
        icon_position.x -= bar_item->alias.size.x;
        bar_right_first_item_x -= bar_item->alias.size.x;
      }
    }
    else if (bar_item->position == BAR_POSITION_CENTER) {
      icon_position.x = bar_center_first_item_x + bar_item->icon_spacing_left;
      label_position.x = icon_position.x + icon->bounds.size.width + bar_item->icon_spacing_right + bar_item->label_spacing_left;

      if (!bar_item->nospace)
        bar_center_first_item_x = label_position.x + label->bounds.size.width + bar_item->label_spacing_right;
      if (bar_item->has_graph) {
        graph_x = bar_item->nospace ? label_position.x + label->bounds.size.width + bar_item->label_spacing_right : bar_center_first_item_x;
        if (!bar_item->nospace)
          bar_center_first_item_x += bar_item->graph_data.graph_width;
      }
      if (bar_item->has_alias) {
        bar_center_first_item_x += bar_item->alias.size.x;
      }
    }
    bar_item->label_line.bounds.origin = label_position;
    bar_item->icon_line.bounds.origin = icon_position;
    bar_item_append_associated_bar(bar_item, (1 << (bar->adid - 1)));
    bar_item_set_bounding_rect_for_space(bar_item, sid, bar->origin);

    bar_draw_item_background(bar, bar_item, sid);
    bar_draw_line(bar, icon, icon_position.x, icon_position.y + bar_item->y_offset);
    bar_draw_line(bar, label, label_position.x, label_position.y + bar_item->y_offset);
    bar_draw_alias(bar, bar_item, icon_position.x);
    bar_draw_graph(bar, bar_item, graph_x, graph_rtl);
  }

  CGContextFlush(bar->context);
  SLSOrderWindow(g_connection, bar->id, 1, bar->id);
  SLSReenableUpdate(g_connection);
}

void bar_create_frame(struct bar *bar, CFTypeRef *frame_region) {
  CGRect bounds = display_bounds(bar->did);
  bounds.size.width -= 2*g_bar_manager.margin;
  CGPoint origin = bounds.origin;
  origin.x += g_bar_manager.margin;
  origin.y += g_bar_manager.y_offset;


  if (0 == strcmp(g_bar_manager.position, BAR_POSITION_BOTTOM)) {
    origin.y = CGRectGetMaxY(bounds) - g_bar_manager.height - 2*g_bar_manager.y_offset;
  } else if (display_menu_bar_visible()) {
    //CGRect menu = display_menu_bar_rect(bar->did);
    //origin.y   += menu.size.height;
  }

  bar->frame = (CGRect) {{0, 0},{bounds.size.width, g_bar_manager.height}};
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

void bar_create_window(struct bar* bar) {
  uint32_t set_tags[2] = {
    kCGSStickyTagBit |
      kCGSModalWindowTagBit |
      kCGSDisableShadowTagBit |
      kCGSHighQualityResamplingTagBit |
      kCGSIgnoreForExposeTagBit
  };

  uint32_t clear_tags[2] = { 0, 0 };
  *((int8_t *)(clear_tags) + 0x5) = 0x20;

  CFTypeRef frame_region;
  bar_create_frame(bar, &frame_region);

  SLSNewWindow(g_connection, 2, bar->origin.x, bar->origin.y, frame_region, &bar->id);
  CFRelease(frame_region);

  SLSSetWindowResolution(g_connection, bar->id, 2.0f);
  SLSSetWindowTags(g_connection, bar->id, set_tags, 64);
  SLSClearWindowTags(g_connection, bar->id, clear_tags, 64);
  SLSSetWindowOpacity(g_connection, bar->id, 0);
  SLSSetWindowBackgroundBlurRadius(g_connection, bar->id, g_bar_manager.blur_radius);
  SLSSetMouseEventEnableFlags(g_connection, bar->id, false);
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
