#include "popup.h"
#include "bar_item.h"
#include "bar_manager.h"

void popup_init(struct popup* popup) {
  popup->drawing = false;
  popup->horizontal = false;
  popup->overrides_cell_size = false;
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
  uint32_t total_item_width = 0;
  uint32_t width = 0;
  uint32_t height = 0;

  if (popup->background.enabled
      && popup->background.image.enabled) {
    width = popup->background.image.bounds.size.width
            + 2*popup->background.border_width;
  }

  if (popup->horizontal) {
    for (int j = 0; j < popup->num_items; j++) {
      struct bar_item* bar_item = popup->items[j];
      if (!bar_item->drawing) continue;
      uint32_t cell_height = bar_item_get_height(bar_item) > popup->cell_size
                             ? bar_item_get_height(bar_item)
                             : popup->cell_size;

      total_item_width += bar_item->background.padding_right
                          + bar_item->background.padding_left
                          + bar_item_get_length(bar_item, false);

      if (cell_height > height && popup->horizontal) height = cell_height;
    }

    if (popup->background.enabled
        && popup->background.image.enabled) {
      if (popup->background.image.bounds.size.height > height)
        height = popup->background.image.bounds.size.height;
      
      x = (width - total_item_width) / 2;
    }

  }

  for (int j = 0; j < popup->num_items; j++) {
    struct bar_item* bar_item = NULL;
    if (popup->horizontal) bar_item = popup->items[j];
    else bar_item = popup->items[popup->num_items - 1 - j];
    if (!bar_item->drawing) continue;
    uint32_t cell_height = bar_item_get_height(bar_item) > popup->cell_size
                           ? bar_item_get_height(bar_item)
                           : popup->cell_size;

    uint32_t item_width = bar_item->background.padding_right
                          + bar_item->background.padding_left
                          + bar_item_calculate_bounds(bar_item,
                                                      popup->horizontal
                                                       ? height
                                                       : cell_height,
                                                      x,
                                                      y + (popup->horizontal
                                                           ? height
                                                           : cell_height) / 2);

    if (popup->adid > 0) {
      struct window* window = bar_item_get_window(bar_item, popup->adid);
      window->origin.x = x + popup->anchor.x;
      window->origin.y = y + popup->anchor.y;
      window->frame.size.height = popup->horizontal ? height : cell_height;
      window->frame.size.width = item_width;
    }

    if (item_width > width && !popup->horizontal) width = item_width;
    if (popup->horizontal) x += item_width;
    else y += cell_height;
  }

  if (popup->horizontal) {
    if (!popup->background.enabled || !popup->background.image.enabled) {
      width = x + popup->background.border_width;
    }
    y += height;
  }
  else if (!popup->background.enabled || !popup->background.image.enabled) {
    width += popup->background.border_width;
  }
  y += popup->background.border_width;

  popup->background.bounds.size.width = width;
  popup->background.bounds.size.height = y;
  popup->background.image.bounds.origin.x = popup->background.border_width;
  popup->background.image.bounds.origin.y = popup->background.border_width;
}

void popup_create_window(struct popup* popup) {
  window_create(&popup->window, (CGRect){{popup->anchor.x, popup->anchor.y},
                                         {popup->background.bounds.size.width,
                                          popup->background.bounds.size.height}});

  if (!popup->background.shadow.enabled)
    window_disable_shadow(&popup->window);

  window_set_level(&popup->window, kCGScreenSaverWindowLevelKey);
  CGContextSetInterpolationQuality(popup->window.context, kCGInterpolationNone);
  context_set_font_smoothing(popup->window.context, g_bar_manager.font_smoothing);

  popup->drawing = true;
}

void popup_close_window(struct popup* popup) {
  window_close(&popup->window);
  popup->drawing = false;
}

void popup_add_item(struct popup* popup, struct bar_item* bar_item) {
  popup->num_items++;
  popup->items = realloc(popup->items,
                         sizeof(struct bar_item*)*popup->num_items);
  popup->items[popup->num_items - 1] = bar_item;
}

bool popup_contains_item(struct popup* popup, struct bar_item* bar_item) {
  for (int i = 0; i < popup->num_items; i++) {
    if (popup->items[i] == bar_item) return true;
  }
  return false;
}

void popup_remove_item(struct popup* popup, struct bar_item* bar_item) {
  if (popup->num_items == 0 || !popup_contains_item(popup, bar_item))
    return;
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
  popup->items = realloc(popup->items,
                         sizeof(struct bar_item*)*popup->num_items);
  memcpy(popup->items, tmp, sizeof(struct bar_item*)*popup->num_items);
}

void popup_set_anchor(struct popup* popup, CGPoint anchor, uint32_t adid) {
  if (popup->anchor.x != anchor.x
      || popup->anchor.y != anchor.y + popup->y_offset) {
    popup->anchor = anchor;
    popup->anchor.y += popup->y_offset;
    SLSMoveWindow(g_connection, popup->window.id, &popup->anchor);
  }
  popup->adid = adid;
}

void popup_set_drawing(struct popup* popup, bool drawing) {
  if (!drawing && popup->drawing) popup_close_window(popup);
  else if (drawing && !popup->drawing) popup_create_window(popup);
}

void popup_draw(struct popup* popup) {
  if (!popup->drawing || popup->adid <= 0) return;

  SLSOrderWindow(g_connection, popup->window.id, -1, 0);
  window_resize(&popup->window, (CGRect){{popup->anchor.x, popup->anchor.y},
                                         {popup->background.bounds.size.width,
                                          popup->background.bounds.size.height}});

  CGContextClearRect(popup->window.context, popup->background.bounds);

  bool shadow = popup->background.shadow.enabled;
  popup->background.shadow.enabled = false;
  background_draw(&popup->background, popup->window.context);
  popup->background.shadow.enabled = shadow;

  for (int i = 0; i < popup->num_items; i++) {
    struct bar_item* bar_item = popup->items[i];
    if (!bar_item->drawing) continue;
    if (bar_item->update_mask & UPDATE_MOUSE_ENTERED
        || bar_item->update_mask & UPDATE_MOUSE_EXITED) {
      CGRect tracking_rect = cgrect_mirror_y(bar_item_construct_bounding_rect(bar_item),
                                             popup->background.bounds.size.height / 2.  );

      tracking_rect.origin.y -= tracking_rect.size.height;
      SLSAddTrackingRect(g_connection, popup->window.id, tracking_rect);
    }

    // bar_item_set_bounding_rect_for_display(bar_item,
    //                                        popup->adid,
    //                                        popup->anchor,
    //                                        popup->background.bounds.size.height);

    bool state = bar_item->popup.drawing;
    bar_item->popup.drawing = false;
    bar_item_draw(bar_item, popup->window.context);
    bar_item->popup.drawing = state;
  }
  CGContextFlush(popup->window.context);
  SLSOrderWindow(g_connection, popup->window.id, 1, popup->window.id);
}

void popup_destroy(struct popup* popup) {
  for (int i = 0; i < popup->num_items; i++) {
    bar_manager_remove_item(&g_bar_manager, popup->items[i]);
  }
  if (popup->items) free(popup->items);
  popup_close_window(popup);
}

bool popup_parse_sub_domain(struct popup* popup, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_YOFFSET)) {
    popup->y_offset = token_to_int(get_token(&message));
    return true;
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    popup_set_drawing(popup,
                      evaluate_boolean_state(get_token(&message),
                      popup->drawing)                            );
    return true;
  } else if (token_equals(property, PROPERTY_HORIZONTAL)) {
    popup->horizontal = evaluate_boolean_state(get_token(&message),
                                               popup->horizontal   );
    return true;
  } else if (token_equals(property, PROPERTY_ALIGN)) {
    popup->align = get_token(&message).text[0];
    return true;
  } else if (token_equals(property, PROPERTY_HEIGHT)) {
    popup->cell_size = token_to_int(get_token(&message));
    popup->overrides_cell_size = true;
    return true;
  } 
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text,
                                                              '.'           );
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
      struct token entry = {key_value_pair.value,strlen(key_value_pair.value)};
      if (token_equals(subdom, SUB_DOMAIN_BACKGROUND))
        return background_parse_sub_domain(&popup->background,
                                           rsp,
                                           entry,
                                           message            );
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
