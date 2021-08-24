#ifndef BAR_ITEM_H
#define BAR_ITEM_H

#define BAR_ITEM      'i'
#define BAR_COMPONENT 'c'
#define BAR_PLUGIN    'p'

#define UPDATE_FRONT_APP_SWITCHED 1
#define UPDATE_SPACE_CHANGE       1 << 1
#define UPDATE_DISPLAY_CHANGE     1 << 2
#define UPDATE_SYSTEM_WOKE        1 << 3

struct bar_item {
  bool enabled;
  bool hidden;
  bool is_shown;
  bool nospace;
  int counter;
  char type;
  char* identifier;
  
  // These are 32bit masks where the ith bit represents the ith screen/display association
  uint32_t associated_display;
  uint32_t associated_space;
  uint32_t update_frequency;

  // Execute with exec_fork, callback from command via messages
  bool cache_scripts;
  char* script;
  char* on_click_script;
  
  // Name by which to refer to the bar_item in the configuration
  char* name;

  // The position in the bar: l,r,c
  char position;

  // Icon properties
  struct bar_line icon_line;
  char* icon;
  char* icon_font_name;
  CTFontRef icon_font;
  uint32_t icon_spacing_left;
  uint32_t icon_spacing_right;
  struct rgba_color icon_color;
  struct rgba_color icon_highlight_color;

  // Label properties
  struct bar_line label_line;
  char* label;
  char* label_font_name;
  CTFontRef label_font;
  uint32_t label_spacing_left;
  uint32_t label_spacing_right;
  struct rgba_color label_color;
  struct rgba_color label_highlight_color;

  // Graph Data
  bool has_graph;
  struct graph_data graph_data;

  // Update Events
  uint32_t update_mask;

  // Bounding Boxes for click events
  uint32_t num_rects;
  CGRect** bounding_rects; 
};

struct bar_item* bar_item_create();
void bar_item_script_update(struct bar_item* bar_item, bool forced);
void bar_item_update_component(struct bar_item* bar_item, uint32_t did, uint32_t sid);
void bar_item_init(struct bar_item* bar_item, struct bar_item* default_item);
void bar_item_set_name(struct bar_item* bar_item, char* name);
void bar_item_set_script(struct bar_item* bar_item, char* script);
void bar_item_set_click_script(struct bar_item* bar_item, char* script);
void bar_item_set_padding_left(struct bar_item* bar_item, uint32_t pad);
void bar_item_set_padding_right(struct bar_item* bar_item, uint32_t pad);
void bar_item_set_icon(struct bar_item* bar_item, char* icon, struct rgba_color color);
void bar_item_set_icon_color(struct bar_item* bar_item, uint32_t color);
void bar_item_set_label(struct bar_item* bar_item, char* label);
void bar_item_set_label_color(struct bar_item* bar_item, uint32_t color);
void bar_item_set_label_font(struct bar_item* bar_item, char *font_string);
void bar_item_set_icon_font(struct bar_item* bar_item, char *font_string);

void bar_item_on_click(struct bar_item* bar_item);

CGRect bar_item_construct_bounding_rect(struct bar_item* bar_item);
void bar_item_set_bounding_rect_for_space(struct bar_item* bar_item, uint32_t sid, CGPoint bar_origin);

#endif
