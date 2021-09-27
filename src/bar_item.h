#ifndef BAR_ITEM_H
#define BAR_ITEM_H

#include <_types/_uint32_t.h>
#define BAR_ITEM            'i'
#define BAR_COMPONENT_GRAPH 'g'
#define BAR_COMPONENT_SPACE 's'
#define BAR_COMPONENT_ALIAS 'a'
#define BAR_PLUGIN          'p'

#define BAR_POSITION_LEFT   'l'
#define BAR_POSITION_RIGHT  'r'
#define BAR_POSITION_CENTER 'c'

#define UPDATE_FRONT_APP_SWITCHED 1
#define UPDATE_SPACE_CHANGE       1 << 1
#define UPDATE_DISPLAY_CHANGE     1 << 2
#define UPDATE_SYSTEM_WOKE        1 << 3

struct bar_item {
  char* name;
  bool needs_update;
  bool lazy;
  bool drawing;
  bool updates;
  bool nospace;
  bool selected;
  int counter;
  char type;
  
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
  struct graph_data graph_data;

  // Alias Data
  bool has_alias;
  struct alias alias;

  // Update Events
  uint32_t update_mask;

  // Bounding Boxes for click events and background drawing (individual per display)
  uint32_t num_rects;
  CGRect** bounding_rects; 
};

struct bar_item* bar_item_create();
void bar_item_init(struct bar_item* bar_item, struct bar_item* default_item);
void bar_item_destroy(struct bar_item* bar_item);

void bar_item_serialize(struct bar_item* bar_item, FILE* rsp);

bool bar_item_update(struct bar_item* bar_item, bool forced);
bool bar_item_is_shown(struct bar_item* bar_item);

void bar_item_append_associated_space(struct bar_item* bar_item, uint32_t bit);
void bar_item_append_associated_display(struct bar_item* bar_item, uint32_t bit);
void bar_item_append_associated_bar(struct bar_item* bar_item, uint32_t bit);
void bar_item_remove_associated_bar(struct bar_item* bar_item, uint32_t bit);
void bar_item_reset_associated_bar(struct bar_item* bar_item);
void bar_item_set_name(struct bar_item* bar_item, char* name);
void bar_item_set_type(struct bar_item* bar_item, char type);
void bar_item_set_script(struct bar_item* bar_item, char* script);
void bar_item_set_click_script(struct bar_item* bar_item, char* script);
void bar_item_set_drawing(struct bar_item* bar_item, bool state);
void bar_item_set_yoffset(struct bar_item* bar_item, int offset);
void bar_item_needs_update(struct bar_item* bar_item);
void bar_item_clear_needs_update(struct bar_item* bar_item);

void bar_item_on_click(struct bar_item* bar_item);

CGRect bar_item_construct_bounding_rect(struct bar_item* bar_item);
void bar_item_set_bounding_rect_for_display(struct bar_item* bar_item, uint32_t adid, CGPoint bar_origin);

#endif
