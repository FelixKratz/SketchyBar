#include "popup.h"
#include "bar_item.h"
#include "bar_manager.h"

void popup_init(struct popup* popup, struct bar_item* host) {
  popup->drawing = false;
  popup->horizontal = false;
  popup->mouse_over = false;
  popup->overrides_cell_size = false;
  popup->needs_ordering = false;
  popup->anchor = (CGPoint){0, 0};
  popup->y_offset = 0;
  popup->adid = 0;
  popup->align = POSITION_LEFT;
  popup->blur_radius = 0;
  
  popup->num_items = 0;
  popup->cell_size = 30;
  popup->items = NULL;
  popup->host = host;
  background_init(&popup->background);
  popup->background.border_color = rgba_color_from_hex(0xffff0000);
  popup->background.color = rgba_color_from_hex(0x44000000);
}

CGRect popup_get_frame(struct popup* popup) {
  return (CGRect){{popup->anchor.x, popup->anchor.y},
                  {popup->background.bounds.size.width,
                   popup->background.bounds.size.height}};
}

bool popup_set_blur_radius(struct popup* popup, uint32_t radius) {
  if (popup->blur_radius == radius) return false;
  popup->blur_radius = radius;
  window_set_blur_radius(&popup->window, radius);
  return true;
}

void popup_order_windows(struct popup* popup) {
  window_set_level(&popup->window, kCGScreenSaverWindowLevel);
  window_order(&popup->window, NULL, W_ABOVE);

  struct window* previous_window = NULL;
  for (int i = 0; i < popup->num_items; i++) {
    struct bar_item* bar_item = popup->items[i];

    struct window* window = bar_item_get_window(bar_item, popup->adid);
    window_set_level(window, kCGScreenSaverWindowLevel);

    if (bar_item->type == BAR_COMPONENT_GROUP) {
      window_order(window, &popup->window, W_ABOVE);
      continue;
    }

    if (previous_window) window_order(window, previous_window, W_ABOVE);
    else window_order(window, &popup->window, W_ABOVE);

    previous_window = window;
  }
}

void popup_calculate_popup_anchor_for_bar_item(struct popup* popup, struct bar_item* bar_item) {
  if (popup->adid != g_bar_manager.active_adid) return;
  struct window* window = bar_item_get_window(bar_item, popup->adid);

  if (!bar_item->popup.overrides_cell_size)
    bar_item->popup.cell_size = window->frame.size.height;

  popup_calculate_bounds(&bar_item->popup);

  CGPoint anchor = window->origin;
  if (bar_item->position != POSITION_POPUP || popup->horizontal) {
    if (bar_item->popup.align == POSITION_CENTER) {
      anchor.x += (window->frame.size.width
                   - bar_item->popup.background.bounds.size.width) / 2;
    } else if (bar_item->popup.align == POSITION_LEFT) {
      anchor.x -= bar_item->background.padding_left;
    } else {
      anchor.x += window->frame.size.width
                  - bar_item->popup.background.bounds.size.width;
    }
    anchor.y += (g_bar_manager.position == POSITION_BOTTOM
                ? (- bar_item->popup.background.bounds.size.height)
                : window->frame.size.height);
  } else if (bar_item->parent) {
    struct popup* host = &bar_item->parent->popup;
    anchor.x = host->window.origin.x;
    if (bar_item->popup.align == POSITION_LEFT) {
      anchor.x -= bar_item->popup.background.bounds.size.width;
    }
    else {
      anchor.x += host->window.frame.size.width;
    }
    anchor.y -= host->background.border_width;
  }
  popup_set_anchor(&bar_item->popup, anchor, popup->adid);
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
      if (bar_item->type == BAR_COMPONENT_GROUP) continue;
      uint32_t cell_height = max(bar_item_get_height(bar_item),
                                 popup->cell_size              );

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
    bar_item = popup->items[j];
    if (!bar_item->drawing) continue;
      if (bar_item->type == BAR_COMPONENT_GROUP) continue;

    uint32_t cell_height = max(bar_item_get_height(bar_item),
                               popup->cell_size              );

    uint32_t item_x = max((int)x + bar_item->background.padding_left, 0);
    uint32_t item_height = popup->horizontal ? height : cell_height;
    uint32_t item_y = item_height / 2;

    uint32_t item_width = bar_item->background.padding_right
                          + bar_item->background.padding_left
                          + bar_item_calculate_bounds(bar_item,
                                                      item_height,
                                                      0,
                                                      item_y      );

    uint32_t bar_item_display_length = bar_item_get_length(bar_item, true);
    if (popup->adid > 0) {
      CGRect frame = {{popup->anchor.x + item_x,
                       popup->anchor.y + y},
                      {bar_item_display_length,
                       item_height             }  };

      window_set_frame(bar_item_get_window(bar_item, popup->adid), frame);

      if (bar_item->group
          && group_is_first_member(bar_item->group, bar_item)) {

        uint32_t group_length = group_get_length(bar_item->group);
        CGRect group_frame = {{frame.origin.x,
                               frame.origin.y },
                              {group_length,
                               frame.size.height}              };
        
        window_set_frame(bar_item_get_window(bar_item->group->members[0],
                                             popup->adid                 ),
                         group_frame                                       );
      }
    }

    if (bar_item->popup.drawing)
      popup_calculate_popup_anchor_for_bar_item(popup, bar_item);

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

  if (popup->adid > 0)
    window_set_frame(&popup->window, popup_get_frame(popup));
}

void popup_create_window(struct popup* popup) {
  window_create(&popup->window,(CGRect){{popup->anchor.x, popup->anchor.y},
                                      {popup->background.bounds.size.width,
                                       popup->background.bounds.size.height}});

  if (!popup->background.shadow.enabled)
    window_disable_shadow(&popup->window);

  CGContextSetInterpolationQuality(popup->window.context,
                                   kCGInterpolationNone);

  context_set_font_smoothing(popup->window.context,
                             g_bar_manager.font_smoothing);

  window_set_blur_radius(&popup->window, popup->blur_radius);
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
  bar_item->parent = popup->host;
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
  popup->anchor = anchor;
  popup->anchor.y += popup->y_offset;

  if ((popup->adid != adid)) {
    popup->needs_ordering = true;
    for (int i = 0; i < popup->num_items; i++) {
      bar_item_needs_update(popup->items[i]);
    }
  }

  popup->adid = adid;
  popup_calculate_bounds(popup);
}

bool popup_set_drawing(struct popup* popup, bool drawing) {
  if (popup->drawing == drawing) return false;
  if (!drawing && popup->drawing) popup_close_window(popup);
  else if (drawing && !popup->drawing) popup_create_window(popup);
  popup->adid = 0;
  return true;
}

void popup_draw(struct popup* popup) {
  if (!popup->drawing || popup->adid <= 0) return;

  window_apply_frame(&popup->window);
  CGContextClearRect(popup->window.context, popup->background.bounds);

  window_assign_mouse_tracking_area(&popup->window, popup->window.frame);

  bool shadow = popup->background.shadow.enabled;
  popup->background.shadow.enabled = false;
  background_draw(&popup->background, popup->window.context);
  popup->background.shadow.enabled = shadow;

  CGContextFlush(popup->window.context);

  if (popup->needs_ordering) {
    popup_order_windows(popup);
    popup->needs_ordering = false;
  }
}

void popup_destroy(struct popup* popup) {
  for (int i = 0; i < popup->num_items; i++) {
    bar_manager_remove_item(&g_bar_manager, popup->items[i]);
  }
  if (popup->items) free(popup->items);
  background_destroy(&popup->background);
  popup_close_window(popup);
}

bool popup_parse_sub_domain(struct popup* popup, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_YOFFSET)) {
    popup->y_offset = token_to_int(get_token(&message));
    return true;
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    return popup_set_drawing(popup,
                             evaluate_boolean_state(get_token(&message),
                             popup->drawing)                            );
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
  } else if (token_equals(property, PROPERTY_BLUR_RADIUS)) {
    popup_set_blur_radius(popup, token_to_int(get_token(&message)));
    return false;
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
        respond(rsp, "[!] Popup: Invalid subdomain '%s'\n", subdom.text);
      }
    }
    else {
      respond(rsp, "[!] Popup: Invalid property '%s'\n", property.text);
    }
  }
  return false;
}
