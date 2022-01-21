#include "popup.h"
#include "background.h"
#include "bar_item.h"
#include "bar_manager.h"
#include "misc/helpers.h"
#include <stdint.h>

void popup_init(struct popup* popup) {
  popup->drawing = false;
  popup->horizontal = false;
  popup->context = NULL;
  popup->id = 0;
  popup->frame.origin = (CGPoint){0,0};
  popup->anchor = (CGPoint){0, 0};
  popup->y_offset = 0;
  popup->adid = 0;
  popup->align = POSITION_LEFT;
  
  popup->num_items = 0;
  popup->cell_size = 30;
  popup->items = NULL;
  background_init(&popup->background);
  popup->background.border_color = rgba_color_from_hex(0xffff0000);
  popup->background.color = rgba_color_from_hex(0x44000000);
}

void popup_calculate_bounds(struct popup* popup) {
  uint32_t y = popup->background.border_width;
  uint32_t x = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  for (int j = 0; j < popup->num_items; j++) {
    struct bar_item* bar_item = NULL;
    if (popup->horizontal) bar_item = popup->items[j];
    else bar_item = popup->items[popup->num_items - 1 - j];
    if (!bar_item->drawing) continue;
    uint32_t cell_height = bar_item_get_height(bar_item) > popup->cell_size ? bar_item_get_height(bar_item) : popup->cell_size;
    uint32_t item_width = bar_item->background.padding_right + bar_item->background.padding_left + bar_item_calculate_bounds(bar_item, cell_height, x, y + cell_height / 2);
    if (item_width > width && !popup->horizontal) width = item_width;
    if (cell_height > height && popup->horizontal) height = cell_height;
    if (popup->horizontal) x += item_width;
    else y += cell_height;
  }

  if (popup->horizontal) {
    width = x;
    y += height;
  }
  y += popup->background.border_width;

  popup->background.bounds.size.width = width;// + popup->background.border_width/2;
  popup->background.bounds.size.height = y;
}

void popup_create_frame(struct popup *popup, CFTypeRef *frame_region) {
  CGSNewRegionWithRect(&popup->background.bounds, frame_region);
}

void popup_resize(struct popup* popup) {
  CFTypeRef frame_region;
  popup_create_frame(popup, &frame_region);

  SLSSetWindowShape(g_connection, popup->id, popup->anchor.x, popup->anchor.y, frame_region);

  SLSClearActivationRegion(g_connection, popup->id);
  SLSAddActivationRegion(g_connection, popup->id, frame_region);
  SLSRemoveAllTrackingAreas(g_connection, popup->id);

  CFRelease(frame_region);
}

void popup_create_window(struct popup* popup) {
  uint64_t set_tags = kCGSStickyTagBit | kCGSHighQualityResamplingTagBit;
  uint64_t clear_tags = kCGSSuperStickyTagBit;

  CFTypeRef frame_region;
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

bool popup_contains_item(struct popup* popup, struct bar_item* bar_item) {
  for (int i = 0; i < popup->num_items; i++) {
    if (popup->items[i] == bar_item) return true;
  }
  return false;
}

void popup_remove_item(struct popup* popup, struct bar_item* bar_item) {
  if (popup->num_items == 0 || !popup_contains_item(popup, bar_item)) return;
  else if (popup->num_items == 1) {
    free(popup->items);
    popup->items = NULL;
    popup->num_items = 0;
    return;
  }

  struct bar_item* tmp[popup->num_items - 1];
  int count = 0;
  for (int i = 0; i < popup->num_items; i++) {
    if (popup->items[i] == bar_item) continue;
    tmp[count++] = popup->items[i];
  }
  popup->num_items--;
  popup->items = realloc(popup->items, sizeof(struct bar_item*)*popup->num_items);
  memcpy(popup->items, tmp, sizeof(struct bar_item*)*popup->num_items);
}


void popup_set_anchor(struct popup* popup, CGPoint anchor, uint32_t adid) {
  if (popup->anchor.x != anchor.x || popup->anchor.y != anchor.y + popup->y_offset) {
    popup->anchor = anchor;
    popup->anchor.y += popup->y_offset;
    SLSMoveWindow(g_connection, popup->id, &popup->anchor);
  }
  popup->adid = adid;
}

void popup_set_drawing(struct popup* popup, bool drawing) {
  if (!drawing && popup->drawing) popup_close_window(popup);
  else if (drawing && !popup->drawing) popup_create_window(popup);
}

void popup_draw(struct popup* popup) {
  if (!popup->drawing) return;

  SLSOrderWindow(g_connection, popup->id, -1, 0);
  popup_resize(popup);
  draw_rect(popup->context, popup->background.bounds, &popup->background.color, popup->background.corner_radius, popup->background.border_width, &popup->background.border_color, true);

  for (int i = 0; i < popup->num_items; i++) {
    struct bar_item* bar_item = popup->items[i];
    if (!bar_item->drawing) continue;
    if (bar_item->update_mask & UPDATE_MOUSE_ENTERED || bar_item->update_mask & UPDATE_MOUSE_EXITED)
      SLSAddTrackingRect(g_connection, popup->id, CGRectInset(bar_item_construct_bounding_rect(bar_item), 1, 1));

    bar_item_set_bounding_rect_for_display(bar_item, popup->adid, popup->anchor, popup->background.bounds.size.height);

    bool state = bar_item->popup.drawing;
    bar_item->popup.drawing = false;
    bar_item_draw(bar_item, popup->context);
    bar_item->popup.drawing = state;
  }
  CGContextFlush(popup->context);
  SLSOrderWindow(g_connection, popup->id, 1, popup->id);
}

void popup_destroy(struct popup* popup) {
  for (int i = 0; i < popup->num_items; i++) {
    bar_manager_remove_item(&g_bar_manager, popup->items[i]);
  }
  if (popup->items) free(popup->items);
  popup_close_window(popup);
}

static bool popup_parse_sub_domain(struct popup* popup, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_YOFFSET)) {
    popup->y_offset = token_to_int(get_token(&message));
    return true;
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    popup_set_drawing(popup, evaluate_boolean_state(get_token(&message), popup->drawing));
    return true;
  } else if (token_equals(property, PROPERTY_HORIZONTAL)) {
    popup->horizontal = evaluate_boolean_state(get_token(&message), popup->horizontal);
    return true;
  } else if (token_equals(property, PROPERTY_ALIGN)) {
    popup->align = get_token(&message).text[0];
    return true;
  } 
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text, '.');
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
      struct token entry = { key_value_pair.value, strlen(key_value_pair.value) };
      if (token_equals(subdom, SUB_DOMAIN_BACKGROUND))
        return background_parse_sub_domain(&popup->background, rsp, entry, message);
      else {
        fprintf(rsp, "Invalid subdomain: %s \n", subdom.text);
        printf("Invalid subdomain: %s \n", subdom.text);
      }
    }
    else {
      fprintf(rsp, "Unknown property: %s \n", property.text);
      printf("Unknown property: %s \n", property.text);
    }
  }
  return false;
}
