#include "popup.h"
#include "bar_item.h"
#include "bar_manager.h"
#include "bar.h"
#include "animation.h"

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
  popup->topmost = true;
  
  popup->num_items = 0;
  popup->cell_size = 30;
  popup->items = NULL;
  popup->host = host;
  background_init(&popup->background);
  window_init(&popup->window);
  color_set_hex(&popup->background.border_color, 0xffff0000);
  color_set_hex(&popup->background.color, 0x44000000);
}

static CGRect popup_get_frame(struct popup* popup) {
  return (CGRect){{popup->anchor.x, popup->anchor.y},
                  {popup->background.bounds.size.width,
                   popup->background.bounds.size.height}};
}

static bool popup_set_blur_radius(struct popup* popup, uint32_t radius) {
  if (popup->blur_radius == radius) return false;
  popup->blur_radius = radius;
  window_set_blur_radius(&popup->window, radius);
  return false;
}

static void popup_order_windows(struct popup* popup) {
  int level = popup->topmost
              ? (kCGPopUpMenuWindowLevel)
              : (kCGBackstopMenuLevel + 1);
  window_set_level(&popup->window, level);
  window_order(&popup->window, NULL, W_ABOVE);

  struct window* previous_window = NULL;
  struct window* first_window = NULL;
  for (int i = 0; i < popup->num_items; i++) {
    struct bar_item* bar_item = popup->items[i];

    struct window* window = bar_item_get_window(bar_item, popup->adid);
    window_set_level(window, level);
    if (!first_window) first_window = window;

    if (bar_item->type == BAR_COMPONENT_GROUP) {
      if (first_window)
        window_order(window, first_window, W_BELOW);
      else
        window_order(window, &popup->window, W_ABOVE);
      continue;
    }

    if (previous_window) window_order(window, previous_window, W_ABOVE);
    else window_order(window, &popup->window, W_ABOVE);

    previous_window = window;
  }
}

static void popup_calculate_popup_anchor_for_bar_item(struct popup* popup, struct bar_item* bar_item, struct bar* bar) {
  if (popup->adid != g_bar_manager.active_adid) return;
  struct window* window = bar_item_get_window(bar_item, popup->adid);

  if (!bar_item->popup.overrides_cell_size)
    bar_item->popup.cell_size = window->frame.size.height;

  popup_calculate_bounds(&bar_item->popup, bar);

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

void popup_calculate_bounds(struct popup* popup, struct bar* bar) {
  uint32_t y = popup->background.border_width;
  uint32_t x = 0;
  uint32_t total_item_width = 0;
  uint32_t width = 0;
  uint32_t height = 0;

  if (popup->background.enabled
      && popup->background.image.enabled) {
    uint32_t image_width = image_get_size(&popup->background.image).width;
    width = image_width + 2*popup->background.border_width;
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
      uint32_t image_height = image_get_size(&popup->background.image).height;
      if (image_height > height) height = image_height;
      
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
    }

    if (bar_item->popup.drawing)
      popup_calculate_popup_anchor_for_bar_item(popup, bar_item, bar);

    if (item_width > width && !popup->horizontal) width = item_width;
    if (popup->horizontal) x += item_width;
    else y += cell_height;
  }

  for (int j = 0; j < popup->num_items; j++) {
    if (popup->adid <= 0) break;
    struct bar_item* bar_item = NULL;
    bar_item = popup->items[j];
    if (!bar_item->drawing) continue;
    if (bar_item->type != BAR_COMPONENT_GROUP) continue;

    uint32_t cell_height = popup->cell_size;
    if (bar_item->group->num_members > 2) {
      cell_height = max(bar_item_get_height(bar_item->group->members[1]),
                        popup->cell_size                                 );
    }

    uint32_t item_height = popup->horizontal ? height : cell_height;
    uint32_t item_y = item_height / 2;

    group_calculate_bounds(bar_item->group, bar, item_y);

    window_set_frame(bar_item_get_window(bar_item->group->members[0],
                                         popup->adid                 ),
                     bar_item->group->bounds                           );
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

  image_calculate_bounds(&popup->background.image,
                         popup->background.border_width,
                         popup->background.border_width
                         + popup->background.image.bounds.size.height / 2);

  if (popup->adid > 0)
    window_set_frame(&popup->window, popup_get_frame(popup));
}

static void popup_create_window(struct popup* popup) {
  popup->drawing = true;

  if (popup == &g_bar_manager.default_item.popup) return;

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
  popup->needs_ordering = true;
}

static void popup_close_window(struct popup* popup) {
  popup->drawing = false;
  if (popup == &g_bar_manager.default_item.popup) return;
  window_close(&popup->window);
}

static bool popup_contains_item(struct popup* popup, struct bar_item* bar_item) {
  for (int i = 0; i < popup->num_items; i++) {
    if (popup->items[i] == bar_item) return true;
  }
  return false;
}

void popup_add_item(struct popup* popup, struct bar_item* bar_item) {
  if (popup_contains_item(popup, bar_item)) return;
  popup->num_items++;
  popup->items = realloc(popup->items,
                         sizeof(struct bar_item*)*popup->num_items);
  popup->items[popup->num_items - 1] = bar_item;
  bar_item->parent = popup->host;
  popup->needs_ordering = true;
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
}

void popup_clear_pointers(struct popup* popup) {
  popup->items = NULL;
  popup->num_items = 0;
  popup->host = NULL;
  window_clear(&popup->window);
}

bool popup_set_drawing(struct popup* popup, bool drawing) {
  if (popup->drawing == drawing) return false;
  if (!drawing) popup_close_window(popup);
  popup->drawing = drawing;
  popup->adid = 0;
  return true;
}

void popup_draw(struct popup* popup) {
  if (!popup->drawing || popup->adid < 1 || popup->num_items == 0) return;

  if (!popup->window.id) popup_create_window(popup);

  window_apply_frame(&popup->window, false);
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

void popup_change_space(struct popup* popup, uint64_t dsid, uint32_t adid) {
  for (int i = 0; i < popup->num_items; i++) {
    struct bar_item* bar_item = popup->items[i];
    bar_item_change_space(bar_item, dsid, adid);
  }

  if (popup->drawing) {
    window_send_to_space(&popup->window, dsid);
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

void popup_serialize(struct popup* popup, char* indent, FILE* rsp) {
  char align[32] = { 0 };
  switch (popup->align) {
    case POSITION_LEFT:
      snprintf(align, 32, "left");
      break;
    case POSITION_RIGHT:
      snprintf(align, 32, "right");
      break;
    case POSITION_CENTER:
      snprintf(align, 32, "center");
      break;
    case POSITION_BOTTOM:
      snprintf(align, 32, "bottom");
      break;
    case POSITION_TOP:
      snprintf(align, 32, "top");
      break;
    default:
      snprintf(align, 32, "invalid");
      break;
  }

  fprintf(rsp, "%s\"drawing\": \"%s\",\n"
               "%s\"horizontal\": \"%s\",\n"
               "%s\"height\": %d,\n"
               "%s\"blur_radius\": %u,\n"
               "%s\"y_offset\": %d,\n"
               "%s\"align\": \"%s\",\n"
               "%s\"background\": {\n",
               indent, format_bool(popup->drawing),
               indent, format_bool(popup->horizontal),
               indent, popup->overrides_cell_size ? popup->cell_size : -1,
               indent, popup->blur_radius,
               indent, popup->y_offset,
               indent, align, indent                                      );

  char deeper_indent[strlen(indent) + 2];
  snprintf(deeper_indent, strlen(indent) + 2, "%s\t", indent);
  background_serialize(&popup->background, deeper_indent, rsp, true);

  fprintf(rsp, "\n%s},\n%s\"items\": [\n", indent, indent);
  for (int i = 0; i < popup->num_items; i++) {
    fprintf(rsp, "%s\t \"%s\"", indent, popup->items[i]->name);
    if (i < popup->num_items - 1) fprintf(rsp, ",\n");
  }
  fprintf(rsp, "\n%s]", indent);
}

static bool popup_set_yoffset(struct popup* popup, int y_offset) {
  if (popup->y_offset == y_offset) return false;
  popup->y_offset = y_offset;
  return true;
}

static bool popup_set_cell_size(struct popup* popup, int size) {
  if (popup->cell_size == size && popup->overrides_cell_size) return false;
  popup->overrides_cell_size = true;
  popup->cell_size = size;
  return true;
}

static bool popup_set_topmost(struct popup* popup, bool topmost) {
  if (topmost == popup->topmost) return false;
  popup->topmost = topmost;
  popup->needs_ordering = true;
  return true;
}

bool popup_parse_sub_domain(struct popup* popup, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_YOFFSET)) {
    ANIMATE(popup_set_yoffset, popup, popup->y_offset, token_to_int(get_token(&message)));
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
    ANIMATE(popup_set_cell_size,
            popup,
            popup->cell_size,
            token_to_int(get_token(&message)));
  } else if (token_equals(property, PROPERTY_BLUR_RADIUS)) {
    ANIMATE(popup_set_blur_radius,
            popup,
            popup->blur_radius,
            token_to_int(get_token(&message)));
    return false;
  } else if (token_equals(property, PROPERTY_TOPMOST)) {
    return popup_set_topmost(popup,
                             evaluate_boolean_state(get_token(&message),
                                                    popup->topmost      ));
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
  return needs_refresh;
}
