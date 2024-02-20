#include "bar.h"
#include "bar_manager.h"
#include "event.h"
#include "display.h"
#include "misc/helpers.h"
#include "window.h"

bool bar_draws_item(struct bar* bar, struct bar_item* bar_item) {
    if (!bar_item->drawing || !bar->shown || bar->hidden) return false;

    if (((bar_item->associated_display > 0
          && (!(bar_item->associated_display & (1 << bar->adid))))
        || (bar_item->associated_to_active_display
            && (bar->adid != g_bar_manager.active_adid)))
        && !bar_item->ignore_association)
      return false;

    if (bar_item->associated_space > 0
        && (!(bar_item->associated_space & (1 << bar->sid))
            && !bar_item->ignore_association)
        && (bar_item->type != BAR_COMPONENT_SPACE)        )
      return false;

    if (bar_item->position == POSITION_POPUP
        && (!bar_item->parent
            || !bar_item->parent->popup.drawing
            || (bar->adid != g_bar_manager.active_adid)))
      return false;

    return true;
}

static void bar_calculate_popup_anchor_for_bar_item(struct bar* bar, struct bar_item* bar_item) {
  if (bar->adid != g_bar_manager.active_adid) return;
  struct window* window = bar_item_get_window(bar_item, bar->adid);

  if (!bar_item->popup.overrides_cell_size) {
    if (g_bar_manager.position == POSITION_LEFT
        || g_bar_manager.position == POSITION_RIGHT) {
      bar_item->popup.cell_size = window->frame.size.width;
    } else {
      bar_item->popup.cell_size = window->frame.size.height;
    }
  }

  popup_calculate_bounds(&bar_item->popup, bar);

  CGPoint anchor = window->origin;

  if (g_bar_manager.position == POSITION_LEFT
      || g_bar_manager.position == POSITION_RIGHT) {
    if (bar_item->popup.align == POSITION_CENTER) {
      anchor.y += (window->frame.size.height
                   - bar_item->popup.background.bounds.size.height) / 2;
    } else if (bar_item->popup.align == POSITION_LEFT) {
      anchor.y -= bar_item->background.padding_left;
    } else {
      anchor.y += window->frame.size.height
                  - bar_item->popup.background.bounds.size.height;
    }
    anchor.x += (g_bar_manager.position == POSITION_RIGHT
                ? (- bar_item->popup.background.bounds.size.width)
                : window->frame.size.width);
  } else {
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
  }
  popup_set_anchor(&bar_item->popup, anchor, bar->adid);
  popup_calculate_bounds(&bar_item->popup, bar);
}

void bar_order_item_windows(struct bar* bar) {
  if (bar->sid < 1 || bar->adid < 1 || !bar->shown) return;
  window_set_level(&bar->window, g_bar_manager.window_level);
  window_order(&bar->window, NULL, W_ABOVE);

  struct window* previous_window = NULL;
  struct window* first_window = NULL;
  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];
    if (bar_item->position == POSITION_POPUP) continue;

    struct window* window = bar_item_get_window(bar_item, bar->adid);
    window_set_level(window, g_bar_manager.window_level);

    if (!first_window) first_window = window;

    if (bar_item->type == BAR_COMPONENT_GROUP) {
      if (first_window)
        window_order(window, first_window, W_BELOW);
      else
        window_order(window, &bar->window, W_ABOVE);
      continue;
    }

    if (previous_window) window_order(window, previous_window, W_ABOVE);
    else window_order(window, &bar->window, W_ABOVE);

    previous_window = window;
  }
}

static void bar_check_for_clip_updates(struct bar* bar) {
  if (!g_bar_manager.bar_needs_update) {
    for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
      struct bar_item* bar_item = g_bar_manager.bar_items[i];
      struct window* window = bar_item_get_window(bar_item, bar->adid);

      bool clips_bar = bar_item_clips_bar(bar_item);
      if (!clips_bar || (!bar_draws_item(bar, bar_item))) {

        if (clips_bar && !CGPointEqualToPoint(window->origin, g_nirvana)) {
          g_bar_manager.bar_needs_update = true;
          break;
        }

        continue;
      }

      g_bar_manager.bar_needs_update
                       |= window->needs_move
                          || window->needs_resize
                          || bar_item_clip_needs_update_for_bar(bar_item, bar);

      if (g_bar_manager.bar_needs_update) break;
    }
  }
}

void bar_draw(struct bar* bar, bool forced) {
  if (bar->sid < 1 || bar->adid < 1) return;

  if (g_bar_manager.might_need_clipping)
    bar_check_for_clip_updates(bar);

  if (g_bar_manager.bar_needs_update) {
    struct background background = g_bar_manager.background;
    background.bounds = bar->window.frame;
    background.bounds.origin.y -= background.y_offset;
    background.shadow.enabled = false;
    background.enabled = true;
    CGContextClearRect(bar->window.context, bar->window.frame);
    background_draw(&background, bar->window.context);
  }

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];
    struct window* window = bar_item_get_window(bar_item, bar->adid);

    if (!bar_draws_item(bar, bar_item)){
      if (!CGPointEqualToPoint(window->origin, g_nirvana)) {
        window_move(window, g_nirvana);
      }

      bar_item_remove_associated_bar(bar_item, bar->adid);
      continue;
    }

    bar_item_append_associated_bar(bar_item, bar->adid);

    if (bar_item->popup.drawing && bar->adid == g_bar_manager.active_adid)
      popup_draw(&bar_item->popup);

    if (g_bar_manager.bar_needs_update) {
      bar_item_clip_bar(bar_item,
                        window->origin.x - bar->window.origin.x,
                        bar                                     );
    }

    if (!window_apply_frame(window, forced) && !bar_item->needs_update)
      continue;

    if (bar_item->update_mask & UPDATE_MOUSE_ENTERED
        || bar_item->update_mask & UPDATE_MOUSE_EXITED) {
      window_assign_mouse_tracking_area(window, window->frame);
    }

    CGContextClearRect(window->context, window->frame);

    bar_item_draw(bar_item, window->context);
    CGContextFlush(window->context);
  }

  if (g_bar_manager.bar_needs_update) CGContextFlush(bar->window.context);
}

static void bar_calculate_bounds_top_bottom(struct bar* bar) {
  bool is_builtin = CGDisplayIsBuiltin(bar->did);
  uint32_t notch_width = is_builtin ? g_bar_manager.notch_width : 0;

  uint32_t center_length = bar_manager_length_for_bar_side(&g_bar_manager,
                                                           bar,
                                                           POSITION_CENTER);

  uint32_t bar_left_first_item_x = max(g_bar_manager.background.padding_left,
                                       0                                     );

  uint32_t bar_right_first_item_x = bar->window.frame.size.width
                                   -max(g_bar_manager.background.padding_right,
                                          0                                  );

  uint32_t bar_center_first_item_x = (bar->window.frame.size.width
                                      - center_length) / 2 - 1;

  uint32_t bar_center_right_first_item_x = (bar->window.frame.size.width
                                            + notch_width) / 2 - 1;

  uint32_t bar_center_left_first_item_x = (bar->window.frame.size.width
                                           - notch_width) / 2 - 1;

  uint32_t* next_position = NULL;
  uint32_t y = bar->window.frame.size.height / 2;

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];

    if (!bar_draws_item(bar, bar_item)
        || bar_item->type == BAR_COMPONENT_GROUP
        || bar_item->position == POSITION_POPUP ) {
      continue;
    } 

    uint32_t bar_item_display_length = bar_item_get_length(bar_item, true);
    bool rtl = false;

    if (bar_item->position == POSITION_LEFT)
      next_position = &bar_left_first_item_x;
    else if (bar_item->position == POSITION_CENTER)
      next_position = &bar_center_first_item_x;
    else if (bar_item->position == POSITION_RIGHT)
      next_position = &bar_right_first_item_x, rtl = true;
    else if (bar_item->position == POSITION_CENTER_RIGHT)
      next_position = &bar_center_right_first_item_x;
    else if (bar_item->position == POSITION_CENTER_LEFT)
      next_position = &bar_center_left_first_item_x, rtl = true;
    else continue;

    if (bar_item->position == POSITION_RIGHT
        || bar_item->position == POSITION_CENTER_LEFT) {
      *next_position = min(*next_position - bar_item_display_length
                           - bar_item->background.padding_right,
                           bar->window.frame.size.width
                           - bar_item_display_length               );
    }
    else {
      *next_position += max((int)-*next_position,
                            bar_item->background.padding_left);
    }

    bar_item->graph.rtl = rtl;

    CGPoint shadow_offsets = bar_item_calculate_shadow_offsets(bar_item);
    uint32_t bar_item_length = bar_item_calculate_bounds(bar_item,
                                 bar->window.frame.size.height
                                 - (g_bar_manager.background.border_width + 1),
                                 max(shadow_offsets.x, 0),
                                 y                                           );

    CGRect frame = {{bar->window.origin.x + *next_position
                    - max(shadow_offsets.x, 0),
                     bar->window.origin.y                 },
                    {bar_item_display_length
                      + shadow_offsets.x
                      + shadow_offsets.y,
                     bar->window.frame.size.height}         };

    window_set_frame(bar_item_get_window(bar_item, bar->adid), frame);

    if (bar_item->popup.drawing)
      bar_calculate_popup_anchor_for_bar_item(bar, bar_item);

    if (bar_item->position == POSITION_RIGHT
        || bar_item->position == POSITION_CENTER_LEFT) {
      *next_position += bar_item->has_const_width
                        ? bar_item_display_length
                          + bar_item->background.padding_right
                          - bar_item->custom_width
                        : (- bar_item->background.padding_left);
    } else {
      *next_position += bar_item->has_const_width
                        ? bar_item->custom_width
                          - bar_item->background.padding_left
                        : (bar_item_length
                           + bar_item->background.padding_right);
    }
  }

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];

    if (bar_item->type != BAR_COMPONENT_GROUP
        || bar_item->position == POSITION_POPUP
        || !bar_draws_item(bar, bar_item)) {
      continue;
    }

    group_calculate_bounds(bar_item->group, bar, y);
    window_set_frame(bar_item_get_window(bar_item->group->members[0],
                                         bar->adid                   ),
                     bar_item->group->bounds                           );

    if (bar_item->popup.drawing)
      bar_calculate_popup_anchor_for_bar_item(bar, bar_item);
  }
}

static void bar_calculate_bounds_left_right(struct bar* bar) {
  uint32_t notch_width = 0;

  uint32_t center_length = bar_manager_length_for_bar_side(&g_bar_manager,
                                                           bar,
                                                           POSITION_CENTER);

  uint32_t bar_left_first_item_y = max(g_bar_manager.background.padding_left,
                                       0                                     );

  uint32_t bar_right_first_item_y = bar->window.frame.size.height
                                   -max(g_bar_manager.background.padding_right,
                                          0                                  );

  uint32_t bar_center_first_item_y = (bar->window.frame.size.height
                                      - 2*g_bar_manager.margin
                                      - center_length) / 2 - 1;

  uint32_t bar_center_right_first_item_y = (bar->window.frame.size.height
                                            + notch_width) / 2 - 1;

  uint32_t bar_center_left_first_item_y = (bar->window.frame.size.height
                                           - notch_width) / 2 - 1;

  uint32_t* next_position = NULL;

  for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];

    if (!bar_draws_item(bar, bar_item)
        || bar_item->type == BAR_COMPONENT_GROUP
        || bar_item->position == POSITION_POPUP ) {
      continue;
    } 

    uint32_t bar_item_display_height = bar_item_get_height(bar_item);
    uint32_t bar_item_display_length = bar_item_get_length(bar_item, true);
    uint32_t x =  0;
    bool rtl = false;

    if (bar_item->position == POSITION_LEFT)
      next_position = &bar_left_first_item_y;
    else if (bar_item->position == POSITION_CENTER)
      next_position = &bar_center_first_item_y;
    else if (bar_item->position == POSITION_RIGHT)
      next_position = &bar_right_first_item_y, rtl = true;
    else if (bar_item->position == POSITION_CENTER_RIGHT)
      next_position = &bar_center_right_first_item_y;
    else if (bar_item->position == POSITION_CENTER_LEFT)
      next_position = &bar_center_left_first_item_y, rtl = true;
    else continue;

    if (bar_item->position == POSITION_RIGHT
        || bar_item->position == POSITION_CENTER_LEFT) {

      *next_position = min(*next_position - bar_item_display_height
                           - bar_item->background.padding_right,
                           bar->window.frame.size.height
                           - bar_item_display_height               );
    }
    else {
      *next_position += max((int)-*next_position,
                            bar_item->background.padding_left);
    }

    bar_item->graph.rtl = rtl;

    CGPoint shadow_offsets = bar_item_calculate_shadow_offsets(bar_item);
    bar_item_calculate_bounds(bar_item,
                              bar_item_display_height,
                              (g_bar_manager.background.bounds.size.height
                               - bar_item_display_length) / 2.
                              + max(shadow_offsets.x, 0),
                              bar_item_display_height / 2.);


    CGRect frame = {{bar->window.origin.x + x
                     - max(shadow_offsets.x, 0),
                     bar->window.origin.y
                     + *next_position
                     + -max(-bar_item->y_offset, 0)},
                    {g_bar_manager.background.bounds.size.height,
                     bar_item_display_height + abs(bar_item->y_offset)}};

    window_set_frame(bar_item_get_window(bar_item, bar->adid), frame);

    if (bar_item->popup.drawing)
      bar_calculate_popup_anchor_for_bar_item(bar, bar_item);

    if (bar_item->position == POSITION_RIGHT
        || bar_item->position == POSITION_CENTER_LEFT) {
      *next_position += bar_item->has_const_width
                        ? bar_item_display_height
                          + bar_item->background.padding_right
                          - bar_item->custom_width
                        : - bar_item->background.padding_left;
    } else {
      *next_position += bar_item->has_const_width
                        ? bar_item->custom_width
                          - bar_item->background.padding_left
                        : (bar_item_display_height
                           + bar_item->background.padding_right);
    }
  }
}

void bar_calculate_bounds(struct bar* bar) {
  if (bar->sid < 1 || bar->adid < 1) return;

  if (g_bar_manager.position == POSITION_LEFT
      || g_bar_manager.position == POSITION_RIGHT) {
    bar_calculate_bounds_left_right(bar);
  } else {
    bar_calculate_bounds_top_bottom(bar);
  }
}

static CGRect bar_get_frame(struct bar *bar) {
  bool is_builtin = CGDisplayIsBuiltin(bar->did);
  int notch_offset = is_builtin ? g_bar_manager.notch_offset : 0;


  CGRect bounds = display_bounds(bar->did);
  CGPoint origin = bounds.origin;

  if (g_bar_manager.position == POSITION_LEFT
      || g_bar_manager.position == POSITION_RIGHT) {
    bounds.size.height -= 2*g_bar_manager.background.y_offset;

    origin.x += (g_bar_manager.position == POSITION_RIGHT
                 ? (bounds.size.width
                    - g_bar_manager.background.bounds.size.height
                    - g_bar_manager.margin)
                 : g_bar_manager.margin);

    origin.y += g_bar_manager.background.y_offset;

    if (display_menu_bar_visible() && !g_bar_manager.topmost) {
      CGRect menu = display_menu_bar_rect(bar->did);
      origin.y += menu.size.height;
      bounds.size.height -= menu.size.height;
    }

    return (CGRect) {{origin.x, origin.y},
                     {g_bar_manager.background.bounds.size.height,
                      bounds.size.height                          }};
  } else {
    bounds.size.width -= 2*g_bar_manager.margin;
    CGPoint origin = bounds.origin;
    origin.x += g_bar_manager.margin;
    origin.y += g_bar_manager.background.y_offset + notch_offset;


    if (g_bar_manager.position == POSITION_BOTTOM) {
      origin.y = CGRectGetMaxY(bounds)
                 - g_bar_manager.background.bounds.size.height
                 - 2*(g_bar_manager.background.y_offset) - notch_offset;
    } else if (display_menu_bar_visible() && !g_bar_manager.topmost) {
      CGRect menu = display_menu_bar_rect(bar->did);
      origin.y += menu.size.height;
    }

    return (CGRect) {{origin.x, origin.y},
                     {bounds.size.width,
                      g_bar_manager.background.bounds.size.height}};
  }
}

void bar_resize(struct bar* bar) {
  if (bar->hidden || !bar->shown) {
    window_move(&bar->window, g_nirvana);
    return;
  };
  window_set_frame(&bar->window, bar_get_frame(bar));
  if (window_apply_frame(&bar->window, false)) {
    window_assign_mouse_tracking_area(&bar->window, bar->window.frame);
    g_bar_manager.bar_needs_update = true;
  }
}

void bar_set_hidden(struct bar* bar, bool hidden) {
  if (bar->hidden == hidden) return;
  bar->hidden = hidden;
  
  if (hidden) window_move(&bar->window, g_nirvana);
  else bar_resize(bar);
}

static void bar_create_window(struct bar* bar) {
  window_init(&bar->window);
  window_create(&bar->window, bar_get_frame(bar));
  window_assign_mouse_tracking_area(&bar->window, bar->window.frame);
  window_set_blur_radius(&bar->window, g_bar_manager.blur_radius);
  if (!g_bar_manager.shadow) window_disable_shadow(&bar->window);

  context_set_font_smoothing(bar->window.context, g_bar_manager.font_smoothing);
}

void bar_change_space(struct bar* bar, uint64_t dsid) {
  if (bar->adid < 1) return;

  for (uint32_t i = 0; i < g_bar_manager.bar_item_count; i++) {
    struct bar_item* bar_item = g_bar_manager.bar_items[i];
    bar_item_change_space(bar_item, dsid, bar->adid);
  }

  window_send_to_space(&bar->window, dsid);
}

struct bar *bar_create(uint32_t did) {
  struct bar *bar = malloc(sizeof(struct bar));
  memset(bar, 0, sizeof(struct bar));
  bar->hidden = false;
  bar->mouse_over = false;
  bar->did = did;
  bar->dsid = display_space_id(did);
  bar->sid = mission_control_index(bar->dsid);
  bar->shown = SLSSpaceGetType(g_connection, bar->dsid) != 4;
  g_bar_manager.bar_needs_update = true;
  bar_create_window(bar);
  return bar;
}

void bar_destroy(struct bar *bar) {
  window_close(&bar->window);
  free(bar);
}
