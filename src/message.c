#include "message.h"
#include "bar_item.h"
#include "bar_manager.h"
#include <_types/_uint32_t.h>
#include <string.h>

extern struct event_loop g_event_loop;
extern struct bar_manager g_bar_manager;
extern bool g_verbose;

//TODO: Add signal setup for bar_item to listen on events and update dynamically

#define DOMAIN_BATCH                                        "batch"
#define COMMAND_BATCH_CONFIG                                "--config"
#define COMMAND_BATCH_SET                                   "--set"
#define COMMAND_BATCH_DEFAULT                               "--default"

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
#define COMMAND_SET_SCRIPTING                               "scripting"
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
#define COMMAND_SET_GRAPH_COLOR                             "graph_color"
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

#define DOMAIN_CONFIG                                       "config"
#define COMMAND_CONFIG_DEBUG_OUTPUT                         "debug_output"
#define COMMAND_CONFIG_BAR_BACKGROUND                       "bar_color"
#define COMMAND_CONFIG_BAR_POSITION                         "position"
#define COMMAND_CONFIG_BAR_HEIGHT                           "height"
#define COMMAND_CONFIG_BAR_YOFFSET                          "y_offset"
#define COMMAND_CONFIG_BAR_MARGIN                           "margin"
#define COMMAND_CONFIG_BAR_CORNER_RADIUS                    "corner_radius"
#define COMMAND_CONFIG_BAR_BLUR_RADIUS                      "blur_radius"
#define COMMAND_CONFIG_BAR_PADDING_LEFT                     "padding_left"
#define COMMAND_CONFIG_BAR_PADDING_RIGHT                    "padding_right"
#define COMMAND_CONFIG_BAR_DISPLAY                          "display"


#define ARGUMENT_COMMON_VAL_ON                              "on"
#define ARGUMENT_COMMON_VAL_OFF                             "off"
#define ARGUMENT_COMMON_NO_SPACE                            "nospace" 

#define BAR_COMPONENT_SPACE                                 "space"
#define BAR_COMPONENT_GRAPH                                 "graph"

#define BAR_POSITION_LEFT                                   'l'
#define BAR_POSITION_RIGHT                                  'r'
#define BAR_POSITION_CENTER                                 'c'

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

static void get_key_value_pair(char *token, char **key, char **value) {
    *key = token;

    while (*token) {
        if (token[0] == '=') break;
        ++token;
    }

    if (*token != '=') {
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

static void daemon_fail(FILE *rsp, char *fmt, ...) {
  if (!rsp) printf(FAILURE_MESSAGE);
  else fprintf(rsp, FAILURE_MESSAGE);

  va_list ap;
  va_start(ap, fmt);
  if (!rsp) printf(fmt, ap);
  else vfprintf(rsp, fmt, ap);
  va_end(ap);
}

#define VIEW_SET_PROPERTY(p) \
  int p_val = 0; \
  if (token_to_int(value, &p_val)) { \
    view->custom_##p = true; \
    view->p = p_val; \
    view_update(view); \
    view_flush(view); \
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
  if (token_equals(state, ARGUMENT_COMMON_VAL_ON))
    g_bar_manager.frozen = true;
  else if (token_equals(state, ARGUMENT_COMMON_VAL_OFF)) 
    g_bar_manager.frozen = false;

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
  if (bar_item->is_shown) bar_manager_refresh(&g_bar_manager, false);
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
    bar_item->type = BAR_ITEM;
    bar_item->identifier = token_to_string(name);
  } else if (token_equals(command, COMMAND_ADD_COMPONENT)) {
    struct token identifier = get_token(&message);
    name = get_token(&message);
    position = get_token(&message);

    bar_item->type = BAR_COMPONENT;
    bar_item->identifier = token_to_string(identifier);
    if (strcmp(bar_item->identifier, BAR_COMPONENT_GRAPH) == 0) {
      struct token width = get_token(&message);
      graph_data_init(&bar_item->graph_data, token_to_uint32t(width));
      if (message != NULL) {
        struct token modifier = get_token(&message);
        if (token_equals(modifier, ARGUMENT_COMMON_NO_SPACE)) {
          bar_item->nospace = true;
        }
      }
      bar_item->has_graph = true;
    }
    else if (strcmp(bar_item->identifier, BAR_COMPONENT_SPACE) == 0) {
      bar_item_set_script(bar_item, string_copy("if [ \"$SELECTED\" = \"true\" ]; then sketchybar -m set $NAME icon_highlight on; else sketchybar -m set $NAME icon_highlight off; fi"));
      bar_item->update_mask |= UPDATE_SPACE_CHANGE;
    }
  } else if (token_equals(command, COMMAND_ADD_PLUGIN)) {
    struct token identifier = get_token(&message);
    name = get_token(&message);
    position = get_token(&message);

    bar_item->type = BAR_PLUGIN;
    bar_item->identifier = token_to_string(identifier);
  }   else {
    printf("Command: %s not found \n", command.text);
  }
  bar_item->position = position.text[0];

  bar_item_set_name(bar_item, string_copy(""));
  if (bar_manager_get_item_index_for_name(&g_bar_manager, name.text) >= 0) {
    bar_manager_destroy_item(&g_bar_manager, bar_item);
    printf("Name: %s already exists... skipping \n", name.text);
    return;
  }
  bar_item_set_name(bar_item, token_to_string(name));
  bar_manager_refresh(&g_bar_manager, true);
}

static void bar_item_parse_set_message(struct bar_item* bar_item, char* message) {
  struct token property = get_token(&message);

  if (token_equals(property, COMMAND_SET_ICON)) {
    bar_item_set_icon(bar_item, string_copy(message), false);
  } else if (token_equals(property, COMMAND_SET_LABEL)) {
    bar_item_set_label(bar_item, string_copy(message), false);
  } else if (token_equals(property, COMMAND_SET_LABEL_COLOR)) {
    bar_item_set_label_color(bar_item, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_ICON_COLOR)) {
    bar_item_set_icon_color(bar_item, token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_SCRIPTING)) {
    bar_item->scripting = token_equals(get_token(&message), ARGUMENT_COMMON_VAL_ON) ? true : false;
  } else if (token_equals(property, COMMAND_SET_DRAWING)) {
    bar_item->drawing = token_equals(get_token(&message), ARGUMENT_COMMON_VAL_ON) ? true : false;
  } else if (token_equals(property, COMMAND_SET_LABEL_HIGHLIGHT)) {
    bar_item->label_highlight = token_equals(get_token(&message), ARGUMENT_COMMON_VAL_ON) ? true : false;
    bar_item_update_label_color(bar_item);
  } else if (token_equals(property, COMMAND_SET_ICON_HIGHLIGHT)) {
    bar_item->icon_highlight = token_equals(get_token(&message), ARGUMENT_COMMON_VAL_ON) ? true : false;
    bar_item_update_icon_color(bar_item);
  } else if (token_equals(property, COMMAND_SET_ICON_FONT)) {
    bar_item_set_icon_font(bar_item, string_copy(message), false);
  } else if (token_equals(property, COMMAND_SET_LABEL_FONT)) {
    bar_item_set_label_font(bar_item, string_copy(message), false);
  } else if (token_equals(property, COMMAND_SET_SCRIPT)) {
    bar_item_set_script(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_CLICK_SCRIPT)) {
    bar_item_set_click_script(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_UPDATE_FREQ)) {
    bar_item->update_frequency = token_to_uint32t(get_token(&message));
  } else if (token_equals(property, COMMAND_SET_GRAPH_COLOR)) {
    bar_item->graph_data.color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_ICON_HIGHLIGHT_COLOR)) {
    bar_item->icon_highlight_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_LABEL_HIGHLIGHT_COLOR)) {
    bar_item->label_highlight_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, COMMAND_SET_POSITION)) {
    bar_item->position = get_token(&message).text[0];
  } else if (token_equals(property, COMMAND_SET_ASSOCIATED_SPACE)) {
    struct token token = get_token(&message);
    for (int i = 0; i < token.length; i++) {
      int sep = -1;
      if (token.text[i] == ',') token.text[i] = '\0', sep = i;
      bar_item->associated_space |= 1 << strtoul(&token.text[sep + 1], NULL, 0);
    }
  } else if (token_equals(property, COMMAND_SET_ASSOCIATED_DISPLAY)) {
    struct token token = get_token(&message);
    for (int i = 0; i < token.length; i++) {
      int sep = -1;
      if (token.text[i] == ',') token.text[i] = '\0', sep = i;
      bar_item->associated_display |= 1 << strtoul(&token.text[sep + 1], NULL, 0);
    }
  } else if (token_equals(property, COMMAND_SET_ICON_PADDING_LEFT)) {
    bar_item->icon_spacing_left = token_to_uint32t(get_token(&message));
  } else if (token_equals(property, COMMAND_SET_ICON_PADDING_RIGHT)) {
    bar_item->icon_spacing_right = token_to_uint32t(get_token(&message));
  } else if (token_equals(property, COMMAND_SET_LABEL_PADDING_LEFT)) {
    bar_item->label_spacing_left = token_to_uint32t(get_token(&message));
  } else if (token_equals(property, COMMAND_SET_LABEL_PADDING_RIGHT)) {
    bar_item->label_spacing_right = token_to_uint32t(get_token(&message));
  } else if (token_equals(property, COMMAND_SET_CACHE_SCRIPTS)) {
    bar_item->cache_scripts = token_equals(get_token(&message), ARGUMENT_COMMON_VAL_ON) ? true : false;
  } else if (token_equals(property, COMMAND_SET_LAZY)) {
    bar_item->lazy = token_equals(get_token(&message), ARGUMENT_COMMON_VAL_ON) ? true : false;
  } else if (token_equals(property, COMMAND_DEFAULT_RESET)) {
    bar_item_init(&g_bar_manager.default_item, NULL);
  }

  // DEPRECATED
  else if (token_equals(property, COMMAND_SET_ENABLED)) {
    struct token value = get_token(&message);
    printf("Command: enabled soon to be deprecated: Use drawing and scripting commands \n");
    bar_item->drawing = token_equals(value, ARGUMENT_COMMON_VAL_ON) ? true : false;
    bar_item->scripting = token_equals(value, ARGUMENT_COMMON_VAL_ON) ? true : false;
  } else if (token_equals(property, COMMAND_SET_HIDDEN)) {
    printf("Command: hidden soon to be deprecated: Use drawing command \n");
    struct token value = get_token(&message);
    bar_item->drawing = !(token_equals(value, ARGUMENT_COMMON_VAL_ON) ? true : false);
  } 
}

// Syntax: sketchybar -m default <property> <value>
static void handle_domain_default(FILE* rsp, struct token domain, char* message) {
  bar_item_parse_set_message(&g_bar_manager.default_item, message);
}

// Syntax: sketchybar -m set <name> <property> <value>
static void handle_domain_set(FILE* rsp, struct token domain, char* message) {
  struct token name  = get_token(&message);

  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  if (item_index_for_name < 0) {
    printf("Name: %s not found in bar items \n", name.text);
    return;
  }
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];

  bar_item_parse_set_message(bar_item, message);
  
  if (bar_item->is_shown) bar_manager_refresh(&g_bar_manager, false);
}

static void handle_domain_config(FILE *rsp, struct token domain, char *message) {
  struct token command  = get_token(&message);

  if (token_equals(command, COMMAND_CONFIG_BAR_BACKGROUND)) {
    struct token value = get_token(&message);
    uint32_t color = token_to_uint32t(value);
    bar_manager_set_background_color(&g_bar_manager, color);
  } else if (token_equals(command, COMMAND_CONFIG_BAR_HEIGHT)) {
    struct token token = get_token(&message);
    bar_manager_set_height(&g_bar_manager, atoi(token.text));
  } else if (token_equals(command, COMMAND_CONFIG_BAR_MARGIN)) {
    struct token token = get_token(&message);
    g_bar_manager.margin = token_to_uint32t(token);
  } else if (token_equals(command, COMMAND_CONFIG_BAR_YOFFSET)) {
    struct token token = get_token(&message);
    g_bar_manager.y_offset = token_to_uint32t(token);
  } else if (token_equals(command, COMMAND_CONFIG_BAR_CORNER_RADIUS)) {
    struct token token = get_token(&message);
    g_bar_manager.corner_radius = token_to_uint32t(token);
  } else if (token_equals(command, COMMAND_CONFIG_BAR_BLUR_RADIUS)) {
    struct token token = get_token(&message);
    bar_manager_set_background_blur(&g_bar_manager, token_to_uint32t(token));
  } else if (token_equals(command, COMMAND_CONFIG_BAR_PADDING_LEFT)) {
    struct token token = get_token(&message);
    bar_manager_set_padding_left(&g_bar_manager, atoi(token.text));
  } else if (token_equals(command, COMMAND_CONFIG_BAR_PADDING_RIGHT)) {
    struct token token = get_token(&message);
    bar_manager_set_padding_right(&g_bar_manager, atoi(token.text));
  } else if (token_equals(command, COMMAND_CONFIG_BAR_DISPLAY)) {
    int length = strlen(message);
    if (length <= 0) {
      fprintf(rsp, "%s\n", g_bar_manager.display);
    } else if ((strcmp(message,BAR_DISPLAY_MAIN_ONLY) == 0) || (strcmp(message,BAR_DISPLAY_ALL) == 0)) {
      bar_manager_set_display(&g_bar_manager, string_copy(message));
    } else {
      daemon_fail(rsp, "value for '%.*s' must be either 'main' or 'all'.\n", command.length, command.text);
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_POSITION)) {
    if (strlen(message) <= 0) {
      fprintf(rsp, "%s\n", g_bar_manager.position);
    } else if (strcmp(message, BAR_POSITION_TOP) != 0 && strcmp(message, BAR_POSITION_BOTTOM) != 0) {
      daemon_fail(rsp, "value for '%.*s' must be either '%s' or '%s'.\n", command.length, command.text, BAR_POSITION_TOP, BAR_POSITION_BOTTOM);
    } else {
      bar_manager_set_position(&g_bar_manager, string_copy(message));
    }
  }
  else {
    daemon_fail(rsp, "unknown command '%.*s' for domain '%.*s'\n", command.length, command.text, domain.length, domain.text);
  }
}

// Syntax: sketchybar -m batch --<key>=<value> ... <key>=<value>
static void handle_domain_batch(FILE* rsp, struct token domain, char* message) {
  struct token command = get_token(&message);

  if (token_equals(command, COMMAND_BATCH_SET)) {
    struct token name  = get_token(&message);
    int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
    if (item_index_for_name < 0) {
      printf("Name: %s not found in bar items \n", name.text);
      return;
    }
    struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];
    struct token token = get_token(&message);
    while (token.text && token.length > 0) {
      char* key = NULL;
      char* value = NULL;
      get_key_value_pair(token.text, &key, &value);
      char* rbr_msg = malloc((strlen(key) + (value ? strlen(value) : 0) + 3) * sizeof(char)); 
      pack_key_value_pair(rbr_msg, key, value);
      if (!key) break;

      bar_item_parse_set_message(bar_item, rbr_msg);
      free(rbr_msg);
      token = get_token(&message);
    }
  } else if (token_equals(command, COMMAND_BATCH_DEFAULT)) {
    struct token token = get_token(&message);
    while (token.text && token.length > 0) {
      char* key = NULL;
      char* value = NULL;
      get_key_value_pair(token.text, &key, &value);
      if (!key) break;
      char* rbr_msg = malloc((strlen(key) + (value ? strlen(value) : 0) + 3) * sizeof(char)); 
      pack_key_value_pair(rbr_msg, key, value);

      handle_domain_default(rsp, domain, rbr_msg);
      free(rbr_msg);
      token = get_token(&message);
    }
  } else if (token_equals(command, COMMAND_BATCH_CONFIG)) {
    struct token token = get_token(&message);
    while (token.text && token.length > 0) {
      char* key = NULL;
      char* value = NULL;
      get_key_value_pair(token.text, &key, &value);
      if (!key) break;
      char* rbr_msg = malloc((strlen(key) + (value ? strlen(value) : 0) + 3) * sizeof(char)); 
      pack_key_value_pair(rbr_msg, key, value);

      if (key && value) handle_domain_config(rsp, domain, rbr_msg);
      free(rbr_msg);
      token = get_token(&message);
    }
  }
  bar_manager_refresh(&g_bar_manager, false);
}



#undef VIEW_SET_PROPERTY

struct selector {
  struct token token;
  bool did_parse;

  union {
    int dir;
    uint32_t did;
    uint64_t sid;
    struct window *window;
  };
};

enum label_type {
  LABEL_SPACE,
};

void handle_message(FILE *rsp, char *message) {
  struct token domain = get_token(&message);

  if (token_equals(domain, DOMAIN_SET)){
    handle_domain_set(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_BATCH)){
    handle_domain_batch(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_PUSH)) {
    handle_domain_push(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_CONFIG)) {
    handle_domain_config(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_ADD)){
    handle_domain_add(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_REMOVE)){
    handle_domain_remove(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_UPDATE)) {
    bar_manager_update_components(&g_bar_manager, true);
    bar_manager_script_update(&g_bar_manager, true);
    bar_manager_refresh(&g_bar_manager, true);
  } else if (token_equals(domain, DOMAIN_SUBSCRIBE)) {
    handle_domain_subscribe(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_DEFAULT)) {
    handle_domain_default(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_TRIGGER)) {
    handle_domain_trigger(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_FREEZE)) {
    handle_domain_freeze(rsp, domain, message);
  } else {
    daemon_fail(rsp, "unknown domain '%.*s'\n", domain.length, domain.text);
  }
}

static SOCKET_DAEMON_HANDLER(message_handler) {
  struct event *event = event_create(&g_event_loop, DAEMON_MESSAGE, message);
  event_loop_post(&g_event_loop, event);
}
