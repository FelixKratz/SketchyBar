#include "message.h"
#include "alias.h"
#include "bar_item.h"
#include "bar_manager.h"
#include "display.h"
#include "misc/helpers.h"
#include <_types/_uint32_t.h>
#include <string.h>

extern struct event_loop g_event_loop;
extern struct bar_manager g_bar_manager;
extern bool g_verbose;

//TODO: Add signal setup for bar_item to listen on events and update dynamically

#define DOMAIN_BATCH                                        "batch"
#define COMMAND_BATCH_CONFIG                                "--config" // Add deprecation notice
#define COMMAND_BATCH_BAR                                   "--bar"
#define COMMAND_BATCH_ADD                                   "--add"
#define COMMAND_BATCH_SET                                   "--set"
#define COMMAND_BATCH_DEFAULT                               "--default"
#define COMMAND_BATCH_SUBSCRIBE                             "--subscribe"

#define DOMAIN_ADD                                          "add"
#define COMMAND_ADD_ITEM                                    "item"                                  
#define COMMAND_ADD_COMPONENT                               "component"
#define COMMAND_ADD_PLUGIN                                  "plugin"
#define COMMAND_ADD_EVENT                                   "event"

#define DOMAIN_REMOVE                                       "remove"

#define DOMAIN_UPDATE                                       "update"
#define DOMAIN_FREEZE                                       "freeze"

#define DOMAIN_PUSH                                         "push"

#define DOMAIN_TRIGGER                                      "trigger"
#define DOMAIN_CLEAR                                        "clear"

#define DOMAIN_DEFAULT                                      "default"
#define COMMAND_DEFAULT_RESET                               "reset"

#define DOMAIN_SET                                          "set"
#define COMMAND_SET_ENABLED                                 "enabled" // TODO: Add deprecation notice
#define COMMAND_SET_HIDDEN                                  "hidden" // TOD0: Add deprecation notice
#define COMMAND_SET_DRAWING                                 "drawing"
#define COMMAND_SET_SCRIPTING                               "scripting" // TODO: Add deprecation notice
#define COMMAND_SET_UPDATES                                 "updates"
#define COMMAND_SET_POSITION                                "position"
#define COMMAND_SET_ASSOCIATED_DISPLAY                      "associated_display"
#define COMMAND_SET_ASSOCIATED_SPACE                        "associated_space"
#define COMMAND_SET_UPDATE_FREQ                             "update_freq"
#define COMMAND_SET_SCRIPT                                  "script"
#define COMMAND_SET_CLICK_SCRIPT                            "click_script"
#define COMMAND_SET_ICON_PADDING_LEFT                       "icon_padding_left"
#define COMMAND_SET_ICON_PADDING_RIGHT                      "icon_padding_right"
#define COMMAND_SET_LABEL_PADDING_LEFT                      "label_padding_left"
#define COMMAND_SET_LABEL_PADDING_RIGHT                     "label_padding_right"
#define COMMAND_SET_ICON                                    "icon"
#define COMMAND_SET_ICON_FONT                               "icon_font"
#define COMMAND_SET_ICON_COLOR                              "icon_color"
#define COMMAND_SET_ICON_HIGHLIGHT_COLOR                    "icon_highlight_color"
#define COMMAND_SET_ICON_HIGHLIGHT                          "icon_highlight"
#define COMMAND_SET_DRAWS_BACKGROUND                        "draws_background"
#define COMMAND_SET_BACKGROUND_COLOR                        "background_color"
#define COMMAND_SET_BACKGROUND_BORDER_COLOR                 "background_border_color"
#define COMMAND_SET_BACKGROUND_HEIGHT                       "background_height"
#define COMMAND_SET_BACKGROUND_CORNER_RADIUS                "background_corner_radius"
#define COMMAND_SET_BACKGROUND_BORDER_WIDTH                 "background_border_width"
#define COMMAND_SET_BACKGROUND_PADDING_LEFT                 "background_padding_left"
#define COMMAND_SET_BACKGROUND_PADDING_RIGHT                "background_padding_right"
#define COMMAND_SET_YOFFSET                                 "y_offset"
#define COMMAND_SET_GRAPH_COLOR                             "graph_color"
#define COMMAND_SET_GRAPH_FILL_COLOR                        "graph_fill_color"
#define COMMAND_SET_GRAPH_LINE_WIDTH                        "graph_line_width"
#define COMMAND_SET_LABEL                                   "label"
#define COMMAND_SET_LABEL_COLOR                             "label_color"
#define COMMAND_SET_LABEL_FONT                              "label_font"
#define COMMAND_SET_LABEL_HIGHLIGHT_COLOR                   "label_highlight_color"
#define COMMAND_SET_LABEL_HIGHLIGHT                         "label_highlight"
#define COMMAND_SET_CACHE_SCRIPTS                           "cache_scripts"
#define COMMAND_SET_LAZY                                    "lazy"

#define DOMAIN_SUBSCRIBE                                    "subscribe"
#define COMMAND_SUBSCRIBE_FRONT_APP_SWITCHED                "front_app_switched"
#define COMMAND_SUBSCRIBE_SPACE_CHANGE                      "space_change"
#define COMMAND_SUBSCRIBE_DISPLAY_CHANGE                    "display_change"
#define COMMAND_SUBSCRIBE_SYSTEM_WOKE                       "system_woke"

#define DOMAIN_CONFIG                                       "config" // Add deprecation notice
#define DOMAIN_BAR                                          "bar"
#define COMMAND_DEBUG_OUTPUT                                "debug_output"
#define COMMAND_BAR_BACKGROUND                              "bar_color"
#define COMMAND_BAR_BORDER_COLOR                            "border_color"
#define COMMAND_BAR_BORDER_WIDTH                            "border_width"
#define COMMAND_BAR_POSITION                                "position"
#define COMMAND_BAR_HEIGHT                                  "height"
#define COMMAND_BAR_YOFFSET                                 "y_offset"
#define COMMAND_BAR_MARGIN                                  "margin"
#define COMMAND_BAR_CORNER_RADIUS                           "corner_radius"
#define COMMAND_BAR_BLUR_RADIUS                             "blur_radius"
#define COMMAND_BAR_PADDING_LEFT                            "padding_left"
#define COMMAND_BAR_PADDING_RIGHT                           "padding_right"
#define COMMAND_BAR_DISPLAY                                 "display"
#define COMMAND_BAR_TOPMOST                                 "topmost"
#define COMMAND_BAR_HIDDEN                                  "hidden"
#define COMMAND_BAR_FONT_SMOOTHING                          "font_smoothing"

#define DOMAIN_QUERY                                        "query"
#define COMMAND_QUERY_DEFAULT_ITEMS                         "default_menu_items"
#define COMMAND_QUERY_ITEM                                  "item"
#define COMMAND_QUERY_BAR                                   "bar"

#define ARGUMENT_COMMON_VAL_ON                              "on"
#define ARGUMENT_COMMON_VAL_TRUE                            "true"
#define ARGUMENT_COMMON_VAL_YES                             "yes"
#define ARGUMENT_COMMON_VAL_OFF                             "off"
#define ARGUMENT_COMMON_VAL_FALSE                           "false"
#define ARGUMENT_COMMON_VAL_NO                              "no"
#define ARGUMENT_COMMON_VAL_TOGGLE                          "toggle"
#define ARGUMENT_COMMON_NO_SPACE                            "nospace" 

#define BAR_DISPLAY_MAIN_ONLY                               "main"
#define BAR_DISPLAY_ALL                                     "all"

#define BAR_POSITION_BOTTOM                                 "bottom"
#define BAR_POSITION_TOP                                    "top"

static bool token_equals(struct token token, char *match) {
  char *at = match;
  for (int i = 0; i < token.length; ++i, ++at) {
    if ((*at == 0) || (token.text[i] != *at)) {
      return false;
    }
  }
  return *at == 0;
}

static char *token_to_string(struct token token) {
  char *result = malloc(token.length + 1);
  if (!result) return NULL;

  memcpy(result, token.text, token.length);
  result[token.length] = '\0';
  return result;
}

static uint32_t token_to_uint32t(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return strtoul(buffer, NULL, 0);
}


static int token_to_int(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return (int) strtol(buffer, NULL, 0);
}

static float token_to_float(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return strtof(buffer, NULL);
}

static struct token get_token(char **message) {
  struct token token;

  token.text = *message;
  while (**message) {
    ++(*message);
  }
  token.length = *message - token.text;

  if ((*message)[0] == '\0' && (*message)[1] != '\0') {
    ++(*message);
  } else {
    // NOTE(koekeishiya): don't go past the null-terminator
  }

  return token;
}

static void get_key_value_pair(char *token, char **key, char **value, char split) {
    *key = token;

    while (*token) {
        if (token[0] == split) break;
        ++token;
    }

    if (*token != split) {
        *key = NULL;
        *value = NULL;
    } else if (token[1]) {
        *token = '\0';
        *value = token + 1;
    } else {
        *token = '\0';
        *value = NULL;
    }
}

static void pack_key_value_pair(char* cursor, char* key, char* value) {
  uint32_t key_len = strlen(key);
  uint32_t val_len = value ? strlen(value) : 0;
  memcpy(cursor, key, key_len);
  cursor += key_len;
  *cursor++ = '\0';
  memcpy(cursor, value, val_len);
  cursor += val_len;
  *cursor++ = '\0';
  *cursor++ = '\0';
}


static bool evaluate_boolean_state(struct token state, bool previous_state) {
  if (token_equals(state, ARGUMENT_COMMON_VAL_ON) || token_equals(state, ARGUMENT_COMMON_VAL_YES) || token_equals(state, ARGUMENT_COMMON_VAL_TRUE)) return true;
  else if (token_equals(state, ARGUMENT_COMMON_VAL_TOGGLE)) return !previous_state;
  else return false;
}

static void bar_item_parse_subscribe_message(struct bar_item* bar_item, char* message) {
  struct token event = get_token(&message);

  while (event.text && event.length > 0) {
    if (token_equals(event, COMMAND_SUBSCRIBE_SYSTEM_WOKE)) {
      bar_item->update_mask |= UPDATE_SYSTEM_WOKE;
    } else if (token_equals(event, COMMAND_SUBSCRIBE_SPACE_CHANGE)) {
      bar_item->update_mask |= UPDATE_SPACE_CHANGE;
    } else if (token_equals(event, COMMAND_SUBSCRIBE_DISPLAY_CHANGE)) {
      bar_item->update_mask |= UPDATE_DISPLAY_CHANGE;
    } else if (token_equals(event, COMMAND_SUBSCRIBE_FRONT_APP_SWITCHED)) {
      bar_item->update_mask |= UPDATE_FRONT_APP_SWITCHED;
    } else {
      bar_item->update_mask |= custom_events_get_flag_for_name(&g_bar_manager.custom_events, event.text);
    }
    event = get_token(&message);
  }
}

// Syntax: sketchybar -m subscribe <name> <event>
static void handle_domain_subscribe(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);

  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  if (item_index_for_name < 0) return;
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];

  bar_item_parse_subscribe_message(bar_item, message); 
}

// Syntax: sketchybar -m freeze <on/off>
static void handle_domain_freeze(FILE* rsp, struct token domain, char* message) {
  struct token state = get_token(&message);
  g_bar_manager.frozen = evaluate_boolean_state(state, g_bar_manager.frozen);
  bar_manager_refresh(&g_bar_manager, true);
}


// Syntax: sketchybar -m trigger <event> 
static void handle_domain_trigger(FILE* rsp, struct token domain, char* message) {
  struct token event = get_token(&message);
  bar_manager_custom_events_trigger(&g_bar_manager, event.text);
}

// Syntax: sketchybar -m push <name> <y>
static void handle_domain_push(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token y = get_token(&message);
  
  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  if (item_index_for_name < 0) return;
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];
  graph_data_push_back(&bar_item->graph_data, token_to_float(y));
  bar_item_needs_update(bar_item);
  if (bar_item_is_shown(bar_item)) bar_manager_refresh(&g_bar_manager, false);
}

// Syntax: sketchybar -m remove <item>
static void handle_domain_remove(FILE* rsp, struct token domain, char* message) {
  struct token command  = get_token(&message);
  if (token_equals(command, COMMAND_ADD_ITEM) || token_equals(command, COMMAND_ADD_COMPONENT)) {
    struct token name = get_token(&message);
    int index = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
    if (index < 0) return;
    bar_manager_destroy_item(&g_bar_manager, g_bar_manager.bar_items[index]);
  }
  bar_manager_refresh(&g_bar_manager, true);
}

// Syntax: sketchybar -m add <item|component|plugin> (<identifier>) <name> <position>
static void handle_domain_add(FILE* rsp, struct token domain, char* message) {
  struct token command  = get_token(&message);

  if (token_equals(command, COMMAND_ADD_EVENT)) {
    struct token event = get_token(&message);
    if (message != NULL) custom_events_append(&g_bar_manager.custom_events, token_to_string(event), token_to_string(get_token(&message)));
    else custom_events_append(&g_bar_manager.custom_events, token_to_string(event), NULL);

    return;
  }

  struct token name;
  struct token position;
  struct bar_item* bar_item = bar_manager_create_item(&g_bar_manager);

  if (token_equals(command, COMMAND_ADD_ITEM)) {
    name = get_token(&message);
    position = get_token(&message);
    bar_item_set_type(bar_item, BAR_ITEM);
  } else if (token_equals(command, COMMAND_ADD_COMPONENT)) {
    struct token identifier = get_token(&message);
    name = get_token(&message);
    position = get_token(&message);

    bar_item_set_type(bar_item, identifier.text[0]);
    if (bar_item->type == BAR_COMPONENT_GRAPH) {
      struct token width = get_token(&message);
      graph_data_init(&bar_item->graph_data, token_to_uint32t(width));
      bar_item->has_graph = true;
    }
    else if (bar_item->type == BAR_COMPONENT_SPACE) {
      bar_item_set_script(bar_item, string_copy("if [ \"$SELECTED\" = \"true\" ]; then sketchybar -m set $NAME icon_highlight on; else sketchybar -m set $NAME icon_highlight off; fi"));
      bar_item->update_mask |= UPDATE_SPACE_CHANGE;
    }
    else if (bar_item->type == BAR_COMPONENT_ALIAS) {
      char* owner = NULL;
      char* nme = NULL;
      char* tmp_name = string_copy(name.text);
      get_key_value_pair(tmp_name, &owner, &nme, ',');
      if (!nme || !owner)
        alias_init(&bar_item->alias, token_to_string(name), NULL);
      else
        alias_init(&bar_item->alias, string_copy(owner), string_copy(nme));
      free(tmp_name);
    }
  } else if (token_equals(command, COMMAND_ADD_PLUGIN)) {
    struct token identifier = get_token(&message);
    name = get_token(&message);
    position = get_token(&message);
    bar_item_set_type(bar_item, identifier.text[0]);
  } else {
    printf("Command: %s not found \n", command.text);
    fprintf(rsp, "Command: %s not found \n", command.text);
  }
  struct token modifier = get_token(&message);
  if (token_equals(modifier, ARGUMENT_COMMON_NO_SPACE)) bar_item->nospace = true;
  bar_item->position = position.text[0];

  bar_item_set_name(bar_item, string_copy(""));
  if (bar_manager_get_item_index_for_name(&g_bar_manager, name.text) >= 0) {
    bar_manager_destroy_item(&g_bar_manager, bar_item);
    printf("Name: %s already exists... skipping \n", name.text);
    fprintf(rsp, "Name: %s already exists... skipping \n", name.text);
    return;
  }
  bar_item_set_name(bar_item, token_to_string(name));
  bar_manager_refresh(&g_bar_manager, true);
}

static void message_parse_set_message_for_bar_item(FILE* rsp, struct bar_item* bar_item, char* message) {
  bool needs_update = false;
  struct token property = get_token(&message);

  if (token_equals(property, COMMAND_SET_ICON)) {
    printf("Message: %s \n", message);
    needs_update = text_set_string(&bar_item->icon, token_to_string(get_token(&message)), false);
  } else if (token_equals(property, COMMAND_SET_LABEL)) {
    needs_update = text_set_string(&bar_item->label, token_to_string(get_token(&message)), false);
  } else if (token_equals(property, COMMAND_SET_LABEL_COLOR)) {
    needs_update = text_set_color(&bar_item->label, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_ICON_COLOR)) {
    needs_update = text_set_color(&bar_item->icon, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_SCRIPTING) || token_equals(property, COMMAND_SET_UPDATES)) {
    bar_item->updates = evaluate_boolean_state(get_token(&message), bar_item->updates);
  } else if (token_equals(property, COMMAND_SET_DRAWING)) {
    bar_item_set_drawing(bar_item, evaluate_boolean_state(get_token(&message), bar_item->drawing));
  } else if (token_equals(property, COMMAND_SET_LABEL_HIGHLIGHT)) {
    bar_item->label.highlight = evaluate_boolean_state(get_token(&message), bar_item->label.highlight);
    needs_update = text_update_color(&bar_item->label);
  } else if (token_equals(property, COMMAND_SET_ICON_HIGHLIGHT)) {
    bar_item->icon.highlight = evaluate_boolean_state(get_token(&message), bar_item->icon.highlight);
    needs_update = text_update_color(&bar_item->icon);
  } else if (token_equals(property, COMMAND_SET_DRAWS_BACKGROUND)) {
    needs_update = background_set_enabled(&bar_item->background, evaluate_boolean_state(get_token(&message), bar_item->background.enabled));
  } else if (token_equals(property, COMMAND_SET_BACKGROUND_HEIGHT)) {
    needs_update = background_set_height(&bar_item->background, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_BACKGROUND_CORNER_RADIUS)) {
    needs_update = background_set_corner_radius(&bar_item->background, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_BACKGROUND_BORDER_WIDTH)) {
    needs_update = background_set_border_width(&bar_item->background, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_ICON_FONT)) {
    needs_update = text_set_font(&bar_item->icon, string_copy(message), false);
  } else if (token_equals(property, COMMAND_SET_LABEL_FONT)) {
    needs_update = text_set_font(&bar_item->label, string_copy(message), false);
  } else if (token_equals(property, COMMAND_SET_SCRIPT)) {
    bar_item_set_script(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_CLICK_SCRIPT)) {
    bar_item_set_click_script(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_UPDATE_FREQ)) {
    bar_item->update_frequency = token_to_uint32t(get_token(&message));
  } else if (token_equals(property, COMMAND_SET_GRAPH_COLOR)) {
    bar_item->graph_data.line_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
    bar_item_needs_update(bar_item);
  } else if (token_equals(property, COMMAND_SET_GRAPH_FILL_COLOR)) {
    bar_item->graph_data.fill_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
    bar_item->graph_data.overrides_fill_color = true;
    needs_update = true;
  } else if (token_equals(property, COMMAND_SET_GRAPH_LINE_WIDTH)) {
    bar_item->graph_data.line_width = token_to_float(get_token(&message));
    needs_update = true;
  } else if (token_equals(property, COMMAND_SET_BACKGROUND_COLOR)) {
    needs_update = background_set_color(&bar_item->background, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_BACKGROUND_BORDER_COLOR)) {
    needs_update = background_set_border_color(&bar_item->background, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_ICON_HIGHLIGHT_COLOR)) {
    bar_item->icon.highlight_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
    needs_update = text_update_color(&bar_item->icon);
  } else if (token_equals(property, COMMAND_SET_LABEL_HIGHLIGHT_COLOR)) {
    bar_item->label.highlight_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
    needs_update = text_update_color(&bar_item->label);
  } else if (token_equals(property, COMMAND_SET_POSITION)) {
    bar_item->position = get_token(&message).text[0];
  } else if (token_equals(property, COMMAND_SET_ASSOCIATED_SPACE)) {
    struct token token = get_token(&message);
    for (int i = 0; i < token.length; i++) {
      int sep = -1;
      if (token.text[i] == ',') token.text[i] = '\0', sep = i;
      bar_item_append_associated_space(bar_item, 1 << strtoul(&token.text[sep + 1], NULL, 0));
    }
  } else if (token_equals(property, COMMAND_SET_ASSOCIATED_DISPLAY)) {
    struct token token = get_token(&message);
    for (int i = 0; i < token.length; i++) {
      int sep = -1;
      if (token.text[i] == ',') token.text[i] = '\0', sep = i;
      bar_item_append_associated_display(bar_item, 1 << strtoul(&token.text[sep + 1], NULL, 0));
    }
  } else if (token_equals(property, COMMAND_SET_ICON_PADDING_LEFT)) {
    bar_item->icon.padding_left = token_to_int(get_token(&message));
    needs_update = true;
  } else if (token_equals(property, COMMAND_SET_ICON_PADDING_RIGHT)) {
    bar_item->icon.padding_right = token_to_int(get_token(&message));
    needs_update = true;
  } else if (token_equals(property, COMMAND_SET_LABEL_PADDING_LEFT)) {
    bar_item->label.padding_left = token_to_int(get_token(&message));
    needs_update = true;
  } else if (token_equals(property, COMMAND_SET_LABEL_PADDING_RIGHT)) {
    bar_item->label.padding_right = token_to_int(get_token(&message));
    needs_update = true;
  } else if (token_equals(property, COMMAND_SET_BACKGROUND_PADDING_LEFT)) {
    bar_item->background.padding_left = token_to_int(get_token(&message));
    needs_update = true;
  } else if (token_equals(property, COMMAND_SET_BACKGROUND_PADDING_RIGHT)) {
    bar_item->background.padding_right = token_to_int(get_token(&message));
    needs_update = true;
  } else if (token_equals(property, COMMAND_SET_YOFFSET)) {
    bar_item_set_yoffset(bar_item, token_to_int(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_CACHE_SCRIPTS)) {
    bar_item->cache_scripts = evaluate_boolean_state(get_token(&message), bar_item->cache_scripts);
  } else if (token_equals(property, COMMAND_SET_LAZY)) {
    bar_item->lazy = evaluate_boolean_state(get_token(&message), bar_item->lazy);
  } else if (token_equals(property, COMMAND_DEFAULT_RESET)) {
    bar_item_init(&g_bar_manager.default_item, NULL);
  }

  // DEPRECATED
  else if (token_equals(property, COMMAND_SET_ENABLED)) {
    printf("Command: enabled soon to be deprecated: Use drawing and scripting commands \n");
    fprintf(rsp, "Command: enabled soon to be deprecated: Use drawing and scripting commands \n");
    bar_item->drawing = evaluate_boolean_state(get_token(&message), bar_item->drawing);
    bar_item->updates = evaluate_boolean_state(get_token(&message), bar_item->updates);
  } else if (token_equals(property, COMMAND_SET_HIDDEN)) {
    printf("Command: hidden soon to be deprecated: Use drawing command \n");
    fprintf(rsp, "Command: hidden soon to be deprecated: Use drawing command \n");
    bar_item->drawing = evaluate_boolean_state(get_token(&message), bar_item->drawing);
  } else {
    fprintf(rsp, "unknown command '%s' for domain 'set'\n", property.text);
    printf("unknown command '%s' for domain 'set'\n", property.text);
  }

  if (needs_update) bar_item_needs_update(bar_item);
}

// Syntax: sketchybar -m default <property> <value>
static void handle_domain_default(FILE* rsp, struct token domain, char* message) {
  message_parse_set_message_for_bar_item(rsp, &g_bar_manager.default_item, message);
}

// Syntax: sketchybar -m set <name> <property> <value>
static void handle_domain_set(FILE* rsp, struct token domain, char* message) {
  struct token name  = get_token(&message);

  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  if (item_index_for_name < 0) {
    fprintf(rsp, "Name: %s not found in bar items \n", name.text);
    printf("Name: %s not found in bar items \n", name.text);
    return;
  }
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];
  message_parse_set_message_for_bar_item(rsp, bar_item, message);
  if (bar_item_is_shown(bar_item)) bar_manager_refresh(&g_bar_manager, false);
}

static void handle_domain_bar(FILE *rsp, struct token domain, char *message) {
  struct token command  = get_token(&message);

  if (token_equals(command, COMMAND_BAR_BACKGROUND)) {
    struct token value = get_token(&message);
    uint32_t color = token_to_uint32t(value);
    bar_manager_set_background_color(&g_bar_manager, color);
  } else if (token_equals(command, COMMAND_BAR_HEIGHT)) {
    struct token token = get_token(&message);
    bar_manager_set_height(&g_bar_manager, atoi(token.text));
  } else if (token_equals(command, COMMAND_BAR_BORDER_WIDTH)) {
    struct token token = get_token(&message);
    bar_manager_set_border_width(&g_bar_manager, atoi(token.text));
  } else if (token_equals(command, COMMAND_BAR_BORDER_COLOR)) {
    struct token token = get_token(&message);
    uint32_t color = token_to_uint32t(token);
    bar_manager_set_border_color(&g_bar_manager, color);
  } else if (token_equals(command, COMMAND_BAR_MARGIN)) {
    struct token token = get_token(&message);
    g_bar_manager.margin = token_to_uint32t(token);
  } else if (token_equals(command, COMMAND_BAR_YOFFSET)) {
    struct token token = get_token(&message);
    g_bar_manager.y_offset = token_to_uint32t(token);
  } else if (token_equals(command, COMMAND_BAR_CORNER_RADIUS)) {
    struct token token = get_token(&message);
    g_bar_manager.corner_radius = token_to_uint32t(token);
  } else if (token_equals(command, COMMAND_BAR_BLUR_RADIUS)) {
    struct token token = get_token(&message);
    bar_manager_set_background_blur(&g_bar_manager, token_to_uint32t(token));
  } else if (token_equals(command, COMMAND_BAR_PADDING_LEFT)) {
    struct token token = get_token(&message);
    bar_manager_set_padding_left(&g_bar_manager, atoi(token.text));
  } else if (token_equals(command, COMMAND_BAR_PADDING_RIGHT)) {
    struct token token = get_token(&message);
    bar_manager_set_padding_right(&g_bar_manager, atoi(token.text));
  } else if (token_equals(command, COMMAND_BAR_FONT_SMOOTHING)) {
    struct token state = get_token(&message);
    bar_manager_set_font_smoothing(&g_bar_manager, evaluate_boolean_state(state, g_bar_manager.font_smoothing));
  } else if (token_equals(command, COMMAND_BAR_HIDDEN)) {
    struct token state = get_token(&message);
    struct token select = get_token(&message);
    uint32_t adid = 0;
    if (!(select.length == 0)) {
      adid = token_equals(select, "current") ? display_arrangement(display_active_display_id()) : atoi(select.text);
      if (adid > 0 && adid <= g_bar_manager.bar_count)
        bar_manager_set_hidden(&g_bar_manager, adid, evaluate_boolean_state(state, g_bar_manager.bars[adid - 1]->hidden));
      else
        printf("No bar on display %u \n", adid);
    } else bar_manager_set_hidden(&g_bar_manager, adid, evaluate_boolean_state(state, g_bar_manager.any_bar_hidden));
  } else if (token_equals(command, COMMAND_BAR_TOPMOST)) {
    struct token token = get_token(&message);
    bar_manager_set_topmost(&g_bar_manager, evaluate_boolean_state(token, g_bar_manager.topmost));
  } else if (token_equals(command, COMMAND_BAR_DISPLAY)) {
    int length = strlen(message);
    if (length <= 0) {
      fprintf(rsp, "%s\n", g_bar_manager.display);
      printf("%s\n", g_bar_manager.display);
    } else if ((strcmp(message,BAR_DISPLAY_MAIN_ONLY) == 0) || (strcmp(message,BAR_DISPLAY_ALL) == 0)) {
      bar_manager_set_display(&g_bar_manager, string_copy(message));
    } else {
      printf("value for '%.*s' must be either 'main' or 'all'.\n", command.length, command.text);
      fprintf(rsp, "value for '%.*s' must be either 'main' or 'all'.\n", command.length, command.text);
    }
  } else if (token_equals(command, COMMAND_BAR_POSITION)) {
    if (strlen(message) <= 0) {
      fprintf(rsp, "%s\n", g_bar_manager.position);
      printf("%s\n", g_bar_manager.position);
    } else if (strcmp(message, BAR_POSITION_TOP) != 0 && strcmp(message, BAR_POSITION_BOTTOM) != 0) {
      printf("value for '%.*s' must be either '%s' or '%s'.\n", command.length, command.text, BAR_POSITION_TOP, BAR_POSITION_BOTTOM);
      fprintf(rsp, "value for '%.*s' must be either '%s' or '%s'.\n", command.length, command.text, BAR_POSITION_TOP, BAR_POSITION_BOTTOM);
    } else {
      bar_manager_set_position(&g_bar_manager, string_copy(message));
    }
  }
  else {
    fprintf(rsp, "unknown command '%s' for domain 'bar'\n", command.text);
    printf("unknown command '%s' for domain 'bar'\n", command.text);
  }
}

static char* reformat_batch_key_value_pair(struct token token) {
  char* key = NULL;
  char* value = NULL;
  get_key_value_pair(token.text, &key, &value, '=');
  if (!key) return NULL;
  char* rbr_msg = malloc((strlen(key) + (value ? strlen(value) : 0) + 3) * sizeof(char)); 
  pack_key_value_pair(rbr_msg, key, value);
  return rbr_msg;
}

static char* get_batch_line(char** message) {
  char* cursor = *message;
  bool end_of_batch = false;
  while (true) {
    if (*cursor == '\0' && *(cursor + 1) == '\0') { end_of_batch = true; break; }
    if (*cursor == '\0' && *(cursor + 1) == '-') break;
    cursor++;
  }
  char* rbr_msg = malloc(sizeof(char) * (cursor - *message + 2));
  memcpy(rbr_msg, *message, sizeof(char) * (cursor - *message + 1));
  *(rbr_msg + (cursor - *message + 1)) = '\0';
  if (end_of_batch) *message = cursor;
  else *message = cursor + 1;
  return rbr_msg;
}
// Syntax: sketchybar -m batch --<mode> <key>=<value> ... <key>=<value> \
//                             --<mode> <key>=<value> ... <key>=<value>
static void handle_domain_batch(FILE* rsp, struct token domain, char* message) {
  bar_manager_freeze(&g_bar_manager);
  struct token command = get_token(&message);
  while (command.text && command.length > 0) {
    if (token_equals(command, COMMAND_BATCH_SET)) {
      struct token name  = get_token(&message);
      int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
      if (item_index_for_name < 0) {
        fprintf(rsp, "Name: %s not found in bar items \n", name.text);
        printf("Name: %s not found in bar items \n", name.text);
        break;
      }
      struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];
      struct token token = get_token(&message);
      while (token.text && token.length > 0) {
        char* rbr_msg = reformat_batch_key_value_pair(token);
        if (!rbr_msg) break;
        message_parse_set_message_for_bar_item(rsp, bar_item, rbr_msg);
        free(rbr_msg);
        if (message && message[0] == '-') break;
        token = get_token(&message);
      }
    } else if (token_equals(command, COMMAND_BATCH_DEFAULT)) {
      struct token token = get_token(&message);
      while (token.text && token.length > 0) {
        char* rbr_msg = reformat_batch_key_value_pair(token);
        if (!rbr_msg) break;
        handle_domain_default(rsp, domain, rbr_msg);
        free(rbr_msg);
        if (message && message[0] == '-') break;
        token = get_token(&message);
      }
    } else if (token_equals(command, COMMAND_BATCH_BAR) || token_equals(command, COMMAND_BATCH_CONFIG)) {
      struct token token = get_token(&message);
      while (token.text && token.length > 0) {
        char* rbr_msg = reformat_batch_key_value_pair(token);
        if (!rbr_msg) break;
        handle_domain_bar(rsp, domain, rbr_msg);
        free(rbr_msg);
        if (message && message[0] == '-') break;
        token = get_token(&message);
      }
    } else if (token_equals(command, COMMAND_BATCH_ADD)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_add(rsp, domain, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, COMMAND_BATCH_SUBSCRIBE)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_subscribe(rsp, domain, rbr_msg);
      free(rbr_msg);
    }
    command = get_token(&message);
  }
  bar_manager_unfreeze(&g_bar_manager);
  bar_manager_refresh(&g_bar_manager, false);
}

static void handle_domain_query(FILE* rsp, struct token domain, char* message) {
  struct token token = get_token(&message);

  if (token_equals(token, COMMAND_QUERY_DEFAULT_ITEMS)) {
    print_all_menu_items(rsp);
  } else if (token_equals(token, COMMAND_QUERY_ITEM)) {
    struct token name  = get_token(&message);
    int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
    if (item_index_for_name < 0) {
      fprintf(rsp, "Name: %s not found in bar items \n", name.text);
      printf("Name: %s not found in bar items \n", name.text);
      return;
    }
    bar_item_serialize(g_bar_manager.bar_items[item_index_for_name], rsp);
  } else if (token_equals(token, COMMAND_QUERY_BAR)) {
    bar_manager_serialize(&g_bar_manager, rsp);
  }

}

void handle_message(FILE *rsp, char *message) {
  struct token domain = get_token(&message);

  if (token_equals(domain, DOMAIN_SET)){
    handle_domain_set(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_BATCH)){
    handle_domain_batch(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_PUSH)) {
    handle_domain_push(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_BAR) || token_equals(domain, DOMAIN_CONFIG)) {
    handle_domain_bar(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_ADD)){
    handle_domain_add(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_REMOVE)){
    handle_domain_remove(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_UPDATE)) {
    bar_manager_update_space_components(&g_bar_manager, true);
    bar_manager_update(&g_bar_manager, true);
    bar_manager_refresh(&g_bar_manager, true);
  } else if (token_equals(domain, DOMAIN_SUBSCRIBE)) {
    handle_domain_subscribe(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_DEFAULT)) {
    handle_domain_default(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_TRIGGER)) {
    handle_domain_trigger(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_FREEZE)) {
    handle_domain_freeze(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_QUERY)) {
    handle_domain_query(rsp, domain, message);
  } else {
    fprintf(rsp, "unknown domain '%.*s'\n", domain.length, domain.text);
    printf("unknown domain '%.*s'\n", domain.length, domain.text);
  }
}

static SOCKET_DAEMON_HANDLER(message_handler) {
  int* _sockfd = malloc(sizeof(int));
  memcpy(_sockfd, &sockfd, sizeof(int));
  struct event *event = event_create(&g_event_loop, DAEMON_MESSAGE, _sockfd);
  event_loop_post(&g_event_loop, event);
}
