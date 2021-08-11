#include "bar.h"

extern struct event_loop g_event_loop;
extern struct bar_manager g_bar_manager;
struct cpu_info cpu_info;

static POWER_CALLBACK(power_handler)
{
  struct event *event = event_create(&g_event_loop, BAR_REFRESH, NULL);
  event_loop_post(&g_event_loop, event);
}

static TIMER_CALLBACK(timer_handler)
{
  struct event *event = event_create(&g_event_loop, BAR_REFRESH, NULL);
  event_loop_post(&g_event_loop, event);
}

static SHELL_TIMER_CALLBACK(shell_timer_handler)
{
  struct event *event = event_create(&g_event_loop, SHELL_REFRESH, NULL);
  event_loop_post(&g_event_loop, event);
}

static CTFontRef bar_create_font(char *cstring)
{
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

static CGPoint bar_align_line(struct bar *bar, struct bar_line line, int align_x, int align_y)
{
  float x = 0, y = 0;

  if (align_x == ALIGN_NONE) {
    x = CGContextGetTextPosition(bar->context).x;
  } else if (align_x == ALIGN_LEFT) {
    x = 20;
  } else if (align_x == ALIGN_CENTER) {
    x = (bar->frame.size.width / 2) - (line.bounds.size.width  / 2);
  } else if (align_x == ALIGN_RIGHT) {
    x = bar->frame.size.width - line.bounds.size.width - 20;
  }

  if (align_y == ALIGN_NONE) {
    y = CGContextGetTextPosition(bar->context).y;
  } else if (align_y == ALIGN_TOP) {
    y = bar->frame.size.height;
  } else if (align_y == ALIGN_CENTER) {
    y = (bar->frame.size.height / 2) - ((line.ascent - line.descent) / 2);
  } else if (align_y == ALIGN_BOTTOM) {
    y = line.descent;
  }

  return (CGPoint) { x, y };
}

static void bar_draw_line(struct bar *bar, struct bar_line line, float x, float y)
{
  CGContextSetRGBFillColor(bar->context, line.color.r, line.color.g, line.color.b, line.color.a);
  CGContextSetTextPosition(bar->context, x, y);
  CTLineDraw(line.line, bar->context);
}

static void bar_destroy_line(struct bar_line line)
{
  CFRelease(line.line);
}

static struct bar_line bar_prepare_line(CTFontRef font, char *cstring, struct rgba_color color)
{
  const void *keys[] = { kCTFontAttributeName, kCTForegroundColorFromContextAttributeName };
  const void *values[] = { font, kCFBooleanTrue };
  CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFStringRef string = CFStringCreateWithCString(NULL, cstring, kCFStringEncodingUTF8);
  CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL, string, attributes);
  CTLineRef line = CTLineCreateWithAttributedString(attr_string);

  CGFloat ascent, descent;
  CTLineGetTypographicBounds(line, &ascent, &descent, NULL);
  CGRect bounds = CTLineGetBoundsWithOptions(line, kCTLineBoundsUseGlyphPathBounds);

  CFRelease(string);
  CFRelease(attributes);
  CFRelease(attr_string);

  return (struct bar_line) {
    .line = line,
      .ascent = ascent,
      .descent = descent,
      .bounds = bounds,
      .color = color
  };
}

void bar_draw_graph_line(struct bar *bar, const float data[], size_t ndata, float x, float y, struct rgba_color *color, bool fill, int sample_width, bool right_to_left) {
  const float height = bar->frame.size.height * 0.9f;

  CGContextSaveGState(bar->context);
  CGContextSetRGBStrokeColor(bar->context, color->r, color->g, color->b, 1.0);
  CGContextSetRGBFillColor(bar->context, color->r, color->g, color->b, 0.2);
  CGContextSetLineWidth(bar->context, fill ? 0.5 : 0.75);
  CGMutablePathRef p = CGPathCreateMutable();
  float start_x = x;
  CGPathMoveToPoint(p, NULL, x, y + data[ndata - 1] * height);
  if (right_to_left) {
    for (int i = ndata - 1; i > 0; --i, x -= sample_width) {
      CGPathAddLineToPoint(p, NULL, x, y + data[i] * height);
    }
  }
  else {
    for (int i = 0; i < ndata; i++, x += sample_width) {
      CGPathAddLineToPoint(p, NULL, x, y + data[i] * height);
    }
  }
  CGContextAddPath(bar->context, p);
  CGContextStrokePath(bar->context);
  if (fill) {
    CGPathAddLineToPoint(p, NULL, x + sample_width, 0);
    CGPathAddLineToPoint(p, NULL, start_x, 0);
    CGPathCloseSubpath(p);
    CGContextAddPath(bar->context, p);
    CGContextFillPath(bar->context);
  }
  CGPathRelease(p);
  CGContextRestoreGState(bar->context);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static char * focused_window_title()
{
  ProcessSerialNumber psn = {};
  _SLPSGetFrontProcess(&psn);

  pid_t pid;
  GetProcessPID(&psn, &pid);

  AXUIElementRef application_ref = AXUIElementCreateApplication(pid);
  if (!application_ref)
    return NULL;

  CFTypeRef window_ref = NULL;
  AXUIElementCopyAttributeValue(application_ref, kAXFocusedWindowAttribute, &window_ref);
  if (!window_ref) {
    CFRelease(application_ref);
    return NULL;
  }

  char *title = NULL;
  CFTypeRef value = NULL;
  AXUIElementCopyAttributeValue(window_ref, kAXTitleAttribute, &value);
  if (value) {
    title = cfstring_copy(value);
    CFRelease(value);
  }

  CFRelease(window_ref);
  CFRelease(application_ref);

  return title;
}
#pragma clang diagnostic pop

static int mission_control_index(uint64_t sid)
{
  uint64_t result = 0;
  int desktop_cnt = 1;

  CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
  int display_spaces_count = CFArrayGetCount(display_spaces_ref);

  for (int i = 0; i < display_spaces_count; ++i) {
    CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
    CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
    int spaces_count = CFArrayGetCount(spaces_ref);

    for (int j = 0; j < spaces_count; ++j) {
      CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
      CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
      CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &result);
      if (sid == result) goto out;

      ++desktop_cnt;
    }
  }

  desktop_cnt = 0;
out:
  CFRelease(display_spaces_ref);
  return desktop_cnt;
}

int bar_get_center_length(struct bar_manager* bar_manager) {
  int total_length = 0;
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    if (bar_item->position == BAR_POSITION_CENTER) {
      total_length += bar_item->label_line.bounds.size.width + bar_item->icon_line.bounds.size.width + bar_item->icon_spacing_right + bar_item->label_spacing_left;
      if (i > 0) {
        total_length += bar_manager->bar_items[i-1]->label_spacing_right + bar_item->icon_spacing_left;
      }
    }
  }
  return total_length;
}

bool bar_draw_graphs(struct bar* bar, struct bar_item* bar_item, uint32_t x, int graph_width, bool right_to_left) {
  if (bar_item->type == BAR_COMPONENT && strcmp(bar_item->identifier, "cpu_graph") == 0) {
    struct rgba_color cpu_user_color = rgba_color_from_hex(0xcccccc);
    struct rgba_color cpu_sys_color = rgba_color_from_hex(0x86a9c4);
    if (!cpu_info.is_running) {
      cpu_info.update_freq = 1;
      cpu_create(&cpu_info);
    }

    
    bar_draw_graph_line(bar, cpu_info.load_avg, CPU_WINDOW_SZ, x, 0, &cpu_user_color, true, graph_width / CPU_WINDOW_SZ, right_to_left);
    bar_draw_graph_line(bar, cpu_info.sys_avg, CPU_WINDOW_SZ, x, 0, &cpu_sys_color, false, graph_width / CPU_WINDOW_SZ, right_to_left);
    return true;
  }
  else if (bar_item->type == BAR_COMPONENT && strcmp(bar_item->identifier, "mem_graph") == 0) {
    struct rgba_color mem_color = rgba_color_from_hex(0xcccccc);
    if (!cpu_info.is_running) {
      cpu_info.update_freq = 1;
      cpu_create(&cpu_info);
    }
    
    bar_draw_graph_line(bar, cpu_info.used_mem, CPU_WINDOW_SZ, x, 0, &mem_color, true, graph_width / CPU_WINDOW_SZ, right_to_left);
  }
  return false;
}

void bar_refresh(struct bar *bar)
{
  SLSDisableUpdate(g_connection);
  SLSOrderWindow(g_connection, bar->id, -1, 0);
  CGContextClearRect(bar->context, bar->frame);
  CGContextSetRGBFillColor(bar->context, g_bar_manager.background_color.r, g_bar_manager.background_color.g, g_bar_manager.background_color.b, g_bar_manager.background_color.a);
  CGContextFillRect(bar->context, bar->frame);
  CGContextStrokePath(bar->context);
  uint32_t did = display_arrangement(bar->did);
  uint32_t sid = mission_control_index(display_space_id(bar->did));
  bar_manager_update_components(&g_bar_manager, did, sid);

  int bar_left_final_item_x = g_bar_manager.padding_left;
  int bar_right_first_item_x = bar->frame.size.width - g_bar_manager.padding_right;
  int bar_center_first_item_x = (bar->frame.size.width - bar_get_center_length(&g_bar_manager)) / 2;
  const int graph_width = 150;

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];
    struct bar_line* label = &bar_item->label_line;
    struct bar_line* icon = &bar_item->icon_line;
    CGPoint icon_position = bar_align_line(bar, *icon, ALIGN_CENTER, ALIGN_CENTER);
    CGPoint label_position = bar_align_line(bar, *label, ALIGN_CENTER, ALIGN_CENTER);


    if(bar_item->associated_display > 0 && bar_item->associated_display != did) continue;
    if(bar_item->type != BAR_COMPONENT && bar_item->associated_space > 0 && bar_item->associated_space != sid) continue;
    
    if (bar_item->position == BAR_POSITION_LEFT) {
      icon_position.x = bar_left_final_item_x + bar_item->icon_spacing_left;
      label_position.x = icon_position.x + icon->bounds.size.width + bar_item->icon_spacing_right + bar_item->label_spacing_left;
      bar_draw_line(bar, *icon, icon_position.x, icon_position.y);
      bar_draw_line(bar, *label, label_position.x, label_position.y);
      bar_left_final_item_x = label_position.x + label->bounds.size.width + bar_item->label_spacing_right;
      if (bar_draw_graphs(bar, bar_item, bar_left_final_item_x, graph_width, false)) {
        bar_left_final_item_x += graph_width;
      }
    }
    else if (bar_item->position == BAR_POSITION_RIGHT) {
      label_position.x = bar_right_first_item_x - label->bounds.size.width - bar_item->label_spacing_right;
      icon_position.x = label_position.x - icon->bounds.size.width - bar_item->icon_spacing_right - bar_item->label_spacing_left;
      bar_draw_line(bar, *icon, icon_position.x, icon_position.y);
      bar_draw_line(bar, *label, label_position.x, label_position.y);

      bar_right_first_item_x = icon_position.x - bar_item->icon_spacing_left;
      if (bar_draw_graphs(bar, bar_item, bar_right_first_item_x, graph_width, true)) {
        bar_right_first_item_x -= graph_width;
      }
    }
    else if (bar_item->position == BAR_POSITION_CENTER) {
      icon_position.x = bar_center_first_item_x + bar_item->icon_spacing_left;
      label_position.x = icon_position.x + icon->bounds.size.width + bar_item->icon_spacing_right + bar_item->label_spacing_left;
      bar_draw_line(bar, *icon, icon_position.x, icon_position.y);
      bar_draw_line(bar, *label, label_position.x, label_position.y);

      bar_center_first_item_x = label_position.x + label->bounds.size.width + bar_item->label_spacing_right;
      if (bar_draw_graphs(bar, bar_item, bar_center_first_item_x, graph_width, false)) {
        bar_center_first_item_x += graph_width;
      }
    }
  }

  CGContextFlush(bar->context);
  SLSOrderWindow(g_connection, bar->id, 1, bar->id);
  SLSReenableUpdate(g_connection);
}

static CGPoint bar_create_frame(struct bar *bar, CFTypeRef *frame_region)
{
  CGRect bounds = display_bounds(bar->did);
  CGPoint origin = bounds.origin;

  if (!display_manager_menu_bar_hidden()) {
    CGRect menu = display_manager_menu_bar_rect(bar->did);
    origin.y   += menu.size.height;
  }

  char *btm = "bottom";
  CGFloat display_bottom = CGRectGetMaxY(bounds);
  if (strcmp(g_bar_manager.position, btm) == 0) {
    origin.y = display_bottom - g_bar_manager.height;
  }

  bar->frame = (CGRect) {{0, 0},{bounds.size.width, g_bar_manager.height}};
  CGSNewRegionWithRect(&bar->frame, frame_region);

  return origin;
}

void bar_resize(struct bar *bar)
{
  CFTypeRef frame_region;
  CGPoint origin = bar_create_frame(bar, &frame_region);

  SLSDisableUpdate(g_connection);
  SLSOrderWindow(g_connection, bar->id, -1, 0);
  SLSSetWindowShape(g_connection, bar->id, origin.x, origin.y, frame_region);
  bar_refresh(bar);
  SLSOrderWindow(g_connection, bar->id, 1, 0);
  SLSReenableUpdate(g_connection);
  CFRelease(frame_region);
}

struct bar *bar_create(uint32_t did)
{
  struct bar *bar = malloc(sizeof(struct bar));
  memset(bar, 0, sizeof(struct bar));
  bar->did = did;

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
  CGPoint origin = bar_create_frame(bar, &frame_region);

  SLSNewWindow(g_connection, 2, origin.x, origin.y, frame_region, &bar->id);
  CFRelease(frame_region);

  SLSSetWindowResolution(g_connection, bar->id, 2.0f);
  SLSSetWindowTags(g_connection, bar->id, set_tags, 64);
  SLSClearWindowTags(g_connection, bar->id, clear_tags, 64);
  SLSSetWindowOpacity(g_connection, bar->id, 0);
  SLSSetMouseEventEnableFlags(g_connection, bar->id, false);
  SLSSetWindowLevel(g_connection, bar->id, CGWindowLevelForKey(4));
  bar->context = SLWindowContextCreate(g_connection, bar->id, 0);

  int refresh_frequency = 1;
  int shell_refresh_frequency = 1;
  bar->power_source = IOPSNotificationCreateRunLoopSource(power_handler, NULL);
  bar->refresh_timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + refresh_frequency, refresh_frequency, 0, 0, timer_handler, NULL);
  bar->shell_refresh_timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + shell_refresh_frequency, shell_refresh_frequency, 0, 0, shell_timer_handler, NULL);

  CFRunLoopAddSource(CFRunLoopGetMain(), bar->power_source, kCFRunLoopCommonModes);
  CFRunLoopAddTimer(CFRunLoopGetMain(), bar->refresh_timer, kCFRunLoopCommonModes);
  CFRunLoopAddTimer(CFRunLoopGetMain(), bar->shell_refresh_timer, kCFRunLoopCommonModes);
  
  bar_refresh(bar);

  return bar;
}

void bar_destroy(struct bar *bar)
{
  CFRunLoopRemoveSource(CFRunLoopGetMain(), bar->power_source, kCFRunLoopCommonModes);
  CFRunLoopSourceInvalidate(bar->power_source);

  CFRunLoopRemoveTimer(CFRunLoopGetMain(), bar->refresh_timer, kCFRunLoopCommonModes);
  CFRunLoopTimerInvalidate(bar->refresh_timer);

  CFRunLoopRemoveTimer(CFRunLoopGetMain(), bar->shell_refresh_timer, kCFRunLoopCommonModes);
  CFRunLoopTimerInvalidate(bar->shell_refresh_timer);

  CGContextRelease(bar->context);
  SLSReleaseWindow(g_connection, bar->id);
  free(bar);
}
