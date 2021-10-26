#ifndef BAR_ITEM_H
#define BAR_ITEM_H

#include <_types/_uint32_t.h>
#define BAR_ITEM            'i'
#define BAR_COMPONENT_GRAPH 'g'
#define BAR_COMPONENT_SPACE 's'
#define BAR_COMPONENT_ALIAS 'a'
#define BAR_COMPONENT_GROUP 'b'
#define BAR_PLUGIN          'p'

#define BAR_POSITION_LEFT   'l'
#define BAR_POSITION_RIGHT  'r'
#define BAR_POSITION_CENTER 'c'

struct bar_item {
  char* name;
  char type;

  // Update Modifiers
  uint32_t counter;
  bool needs_update;
  bool updates;
  bool updates_only_when_shown;
  bool lazy;
  bool selected;
  bool mouse_over;

  // Drawing Modifiers
  bool drawing;
  bool has_const_width;
  uint32_t custom_width;
  bool queued_for_redraw;
  bool redraw_in_place;

  // These are 32bit masks where the ith bit represents the ith screen/display/bar association
  uint32_t associated_bar;
  uint32_t associated_display;
  uint32_t associated_space;
  uint32_t update_frequency;

  bool cache_scripts;
  char* script;
  char* click_script;
  struct signal_args signal_args;
  
  // The position in the bar: l,r,c
  char position;
  int y_offset;

  // Background
  struct background background;

  // Icon properties
  struct text icon;

  // Label properties
  struct text label;

  // Graph Data
  bool has_graph;
  struct graph graph;

  uint32_t sandwich_position;
  // Alias Data
  bool has_alias;
  struct alias alias;

  // Group Properties
  struct group* group;

  // Update Events
  uint32_t update_mask;

  // Bounding Boxes for click events and background drawing (individual per display)
  uint32_t num_rects;
  CGRect** bounding_rects;
};

struct bar_item* bar_item_create();
void bar_item_inherit_from_item(struct bar_item* bar_item, struct bar_item* ancestor);
void bar_item_init(struct bar_item* bar_item, struct bar_item* default_item);
void bar_item_destroy(struct bar_item* bar_item);

void bar_item_serialize(struct bar_item* bar_item, FILE* rsp);

bool bar_item_update(struct bar_item* bar_item, char* sender, bool forced);
bool bar_item_is_shown(struct bar_item* bar_item);

void bar_item_append_associated_space(struct bar_item* bar_item, uint32_t bit);
void bar_item_append_associated_display(struct bar_item* bar_item, uint32_t bit);
void bar_item_append_associated_bar(struct bar_item* bar_item, uint32_t adid);
void bar_item_remove_associated_bar(struct bar_item* bar_item, uint32_t adid);
void bar_item_reset_associated_bar(struct bar_item* bar_item);
void bar_item_set_name(struct bar_item* bar_item, char* name);
void bar_item_set_type(struct bar_item* bar_item, char type);
void bar_item_set_script(struct bar_item* bar_item, char* script);
void bar_item_set_click_script(struct bar_item* bar_item, char* script);
void bar_item_set_drawing(struct bar_item* bar_item, bool state);
void bar_item_set_yoffset(struct bar_item* bar_item, int offset);

uint32_t bar_item_get_length(struct bar_item* bar_item);
uint32_t bar_item_get_height(struct bar_item* bar_item);

void bar_item_needs_update(struct bar_item* bar_item);
void bar_item_clear_needs_update(struct bar_item* bar_item);

void bar_item_on_click(struct bar_item* bar_item, uint32_t modifier);
void bar_item_mouse_entered(struct bar_item* bar_item);
void bar_item_mouse_exited(struct bar_item* bar_item);

void bar_item_remove_bounding_rect_for_display(struct bar_item* bar_item, uint32_t adid);
CGRect bar_item_construct_bounding_rect(struct bar_item* bar_item);
void bar_item_set_bounding_rect_for_display(struct bar_item* bar_item, uint32_t adid, CGPoint bar_origin);

#endif
