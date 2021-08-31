#include "message.h"
#include "bar_item.h"
#include "bar_manager.h"

extern struct event_loop g_event_loop;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern struct bar_manager g_bar_manager;
extern bool g_verbose;

//TODO: Add signal setup for bar_item to listen on events and update dynamically

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
#define COMMAND_SET_ENABLED                                 "enabled"
#define COMMAND_SET_HIDDEN                                  "hidden"
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
#define COMMAND_SET_GRAPH_COLOR                             "graph_color"
#define COMMAND_SET_LABEL                                   "label"
#define COMMAND_SET_LABEL_COLOR                             "label_color"
#define COMMAND_SET_LABEL_FONT                              "label_font"
#define COMMAND_SET_CACHE_SCRIPTS                           "cache_scripts"

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

static bool token_equals(struct token token, char *match)
{
  char *at = match;
  for (int i = 0; i < token.length; ++i, ++at) {
    if ((*at == 0) || (token.text[i] != *at)) {
      return false;
    }
  }
  return *at == 0;
}

static bool token_is_valid(struct token token)
{
  return token.text && token.length > 0;
}

static char *token_to_string(struct token token)
{
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
  return   strtoul(buffer, NULL, 0);
}

static float token_to_float(struct token token)
{
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return strtof(buffer, NULL);
}

static struct token get_token(char **message)
{
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

static void daemon_fail(FILE *rsp, char *fmt, ...)
{
  if (!rsp) return;

  fprintf(rsp, FAILURE_MESSAGE);

  va_list ap;
  va_start(ap, fmt);
  vfprintf(rsp, fmt, ap);
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

// Syntax: sketchybar -m subscribe <name> <event>
static void handle_domain_subscribe(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token event = get_token(&message);

  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, token_to_string(name));
  if (item_index_for_name < 0) return;
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];

  if (token_equals(event, COMMAND_SUBSCRIBE_SYSTEM_WOKE)) {
    bar_item->update_mask |= UPDATE_SYSTEM_WOKE;
  } else if (token_equals(event, COMMAND_SUBSCRIBE_SPACE_CHANGE)) {
    bar_item->update_mask |= UPDATE_SPACE_CHANGE;
  } else if (token_equals(event, COMMAND_SUBSCRIBE_DISPLAY_CHANGE)) {
    bar_item->update_mask |= UPDATE_DISPLAY_CHANGE;
  } else if (token_equals(event, COMMAND_SUBSCRIBE_FRONT_APP_SWITCHED)) {
    bar_item->update_mask |= UPDATE_FRONT_APP_SWITCHED;
  } else {
    bar_item->update_mask |= custom_events_get_flag_for_name(&g_bar_manager.custom_events, token_to_string(event));
  }
}

// Syntax: sketchybar -m freeze <on/off>
static void handle_domain_freeze(FILE* rsp, struct token domain, char* message) {
  struct token state = get_token(&message);
  if (token_equals(state, ARGUMENT_COMMON_VAL_ON))
    g_bar_manager.frozen = true;
  else if (token_equals(state, ARGUMENT_COMMON_VAL_OFF)) 
    g_bar_manager.frozen = false;

  bar_manager_refresh(&g_bar_manager);
}


// Syntax: sketchybar -m trigger <event> 
static void handle_domain_trigger(FILE* rsp, struct token domain, char* message) {
  struct token event = get_token(&message);
  bar_manager_custom_events_trigger(&g_bar_manager, token_to_string(event));
}


// Syntax: sketchybar -m default <property> <value>
static void handle_domain_default(FILE* rsp, struct token domain, char* message) {
  struct token property = get_token(&message);
  struct bar_item* bar_item = &g_bar_manager.default_item;
  if (token_equals(property, COMMAND_SET_ICON_FONT)) {
    bar_item_set_icon_font(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_LABEL_FONT)) {
    bar_item_set_label_font(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_UPDATE_FREQ)) {
    struct token value = get_token(&message);
    bar_item->update_frequency = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_LABEL_COLOR)) {
    struct token value = get_token(&message);
    bar_item_set_label_color(bar_item, token_to_uint32t(value));
  } else if (token_equals(property, COMMAND_SET_ICON_COLOR)) {
    struct token value = get_token(&message);
    bar_item_set_icon_color(bar_item, token_to_uint32t(value));
  } else if (token_equals(property, COMMAND_SET_ICON_PADDING_LEFT)) {
    struct token value = get_token(&message);
    bar_item->icon_spacing_left = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_ICON_PADDING_RIGHT)) {
    struct token value = get_token(&message);
    bar_item->icon_spacing_right = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_LABEL_PADDING_LEFT)) {
    struct token value = get_token(&message);
    bar_item->label_spacing_left = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_LABEL_PADDING_RIGHT)) {
    struct token value = get_token(&message);
    bar_item->label_spacing_right = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_CACHE_SCRIPTS)) {
    struct token value = get_token(&message);
    bar_item->cache_scripts = token_equals(value, ARGUMENT_COMMON_VAL_ON) ? true : false;
  } else if (token_equals(property, COMMAND_DEFAULT_RESET)) {
    bar_item_init(&g_bar_manager.default_item, NULL);
  }

}
// Syntax: sketchybar -m push <name> <y>
static void handle_domain_push(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token y = get_token(&message);
  
  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, token_to_string(name));
  if (item_index_for_name < 0) return;
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];
  graph_data_push_back(&bar_item->graph_data, token_to_float(y));
  if (bar_item->is_shown)
    bar_manager_refresh(&g_bar_manager);
}

static void handle_domain_remove(FILE* rsp, struct token domain, char* message) {
  struct token command  = get_token(&message);
  if (token_equals(command, COMMAND_ADD_ITEM) || token_equals(command, COMMAND_ADD_COMPONENT)) {
    struct token name = get_token(&message);
    int index = bar_manager_get_item_index_for_name(&g_bar_manager, token_to_string(name));
    if (index < 0) return;
    bar_manager_destroy_item(&g_bar_manager, g_bar_manager.bar_items[index]);
  }
  bar_manager_refresh(&g_bar_manager);
}
// Syntax: sketchybar -m add <item|component|plugin> (<identifier>) <name> <position>
static void handle_domain_add(FILE* rsp, struct token domain, char* message) {
  struct token command  = get_token(&message);

  if (token_equals(command, COMMAND_ADD_EVENT)) {
    struct token event = get_token(&message);
    if (message != NULL) {
      struct token notification = get_token(&message);
      custom_events_append(&g_bar_manager.custom_events, string_copy(token_to_string(event)), string_copy(token_to_string(notification)));
    }
    else
      custom_events_append(&g_bar_manager.custom_events, string_copy(token_to_string(event)), NULL);

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
    if (token_equals(identifier, "title")) printf("Title component deprecated please use windowTitle.sh script");
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
  } else if (token_equals(command, COMMAND_ADD_PLUGIN)) {
    struct token identifier = get_token(&message);
    name = get_token(&message);
    position = get_token(&message);

    bar_item->type = BAR_PLUGIN;
    bar_item->identifier = token_to_string(identifier);
  }   else {
    exit(1);
  }
  bar_item->position = token_to_string(position)[0];

  bar_item_set_name(bar_item, string_copy(""));
  if (bar_manager_get_item_index_for_name(&g_bar_manager, token_to_string(name)) >= 0) {
    bar_manager_destroy_item(&g_bar_manager, bar_item);
    printf("Name: %s already exists... skipping \n", token_to_string(name));
    return;
  }
  bar_item_set_name(bar_item, string_copy(token_to_string(name)));

  bar_manager_refresh(&g_bar_manager);
}

// Syntax: sketchybar -m set <name> <property> <value>
static void handle_domain_set(FILE* rsp, struct token domain, char* message) {
  struct token name  = get_token(&message);
  struct token property = get_token(&message);

  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, token_to_string(name));
  if (item_index_for_name < 0) {
    printf("Name: %s not found in bar items \n", token_to_string(name));
    return;
  }
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];

  if (token_equals(property, COMMAND_SET_ICON)) {
    bar_item_set_icon(bar_item, string_copy(message), bar_item->icon_color);
  } else if (token_equals(property, COMMAND_SET_ICON_FONT)) {
    bar_item_set_icon_font(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_LABEL)) {
    bar_item_set_label(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_LABEL_FONT)) {
    bar_item_set_label_font(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_SCRIPT)) {
    bar_item_set_script(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_CLICK_SCRIPT)) {
    bar_item_set_click_script(bar_item, string_copy(message));
  } else if (token_equals(property, COMMAND_SET_UPDATE_FREQ)) {
    struct token value = get_token(&message);
    bar_item->update_frequency = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_LABEL_COLOR)) {
    struct token value = get_token(&message);
    bar_item_set_label_color(bar_item, token_to_uint32t(value));
  } else if (token_equals(property, COMMAND_SET_ICON_COLOR)) {
    struct token value = get_token(&message);
    bar_item_set_icon_color(bar_item, token_to_uint32t(value));
  } else if (token_equals(property, COMMAND_SET_GRAPH_COLOR)) {
    struct token value = get_token(&message);
    bar_item->graph_data.color = rgba_color_from_hex(token_to_uint32t(value));
  } else if (token_equals(property, COMMAND_SET_ICON_HIGHLIGHT_COLOR)) {
    struct token value = get_token(&message);
    bar_item->icon_highlight_color = rgba_color_from_hex(token_to_uint32t(value));
  } else if (token_equals(property, COMMAND_SET_POSITION)) {
    struct token value = get_token(&message);
    bar_item->position = token_to_string(value)[0];
  } else if (token_equals(property, COMMAND_SET_ASSOCIATED_SPACE)) {
    struct token value = get_token(&message);
    bar_item->associated_space |= 1 << token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_ASSOCIATED_DISPLAY)) {
    struct token value = get_token(&message);
    bar_item->associated_display |= 1 << token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_ICON_PADDING_LEFT)) {
    struct token value = get_token(&message);
    bar_item->icon_spacing_left = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_ICON_PADDING_RIGHT)) {
    struct token value = get_token(&message);
    bar_item->icon_spacing_right = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_LABEL_PADDING_LEFT)) {
    struct token value = get_token(&message);
    bar_item->label_spacing_left = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_LABEL_PADDING_RIGHT)) {
    struct token value = get_token(&message);
    bar_item->label_spacing_right = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_CACHE_SCRIPTS)) {
    struct token value = get_token(&message);
    bar_item->cache_scripts = token_equals(value, ARGUMENT_COMMON_VAL_ON) ? true : false;
  } else if (token_equals(property, COMMAND_SET_ENABLED)) {
    struct token value = get_token(&message);
    bar_item->enabled = token_equals(value, ARGUMENT_COMMON_VAL_ON) ? true : false;
  } else if (token_equals(property, COMMAND_SET_HIDDEN)) {
    struct token value = get_token(&message);
    bar_item->hidden = token_equals(value, ARGUMENT_COMMON_VAL_ON) ? true : false;
  } 

  if (bar_item->is_shown)
    bar_manager_refresh(&g_bar_manager);
}

static void handle_domain_config(FILE *rsp, struct token domain, char *message)
{
  struct token command  = get_token(&message);

  if (token_equals(command, COMMAND_CONFIG_DEBUG_OUTPUT)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "%s\n", bool_str[g_verbose]);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
      g_verbose = false;
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
      g_verbose = true;
    } else {
      daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_BACKGROUND)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "0x%x\n", g_bar_manager.background_color.p);
    } else {
      uint32_t color = token_to_uint32t(value);
      if (color) {
        bar_manager_set_background_color(&g_bar_manager, color);
      } else {
        daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
      }
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_HEIGHT)) {
    struct token token = get_token(&message);
    if (!token_is_valid(token)) {
      fprintf(rsp, "%"PRIu32"\n", g_bar_manager.height ? g_bar_manager.height : 0);
    } else {
      bar_manager_set_height(&g_bar_manager, atoi(token_to_string(token)));
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_PADDING_LEFT)) {
    struct token token = get_token(&message);
    if (!token_is_valid(token)) {
      fprintf(rsp, "%"PRIu32"\n", g_bar_manager.padding_left ? g_bar_manager.padding_left : 0);
    } else {
      bar_manager_set_padding_left(&g_bar_manager, atoi(token_to_string(token)));
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_PADDING_RIGHT)) {
    struct token token = get_token(&message);
    if (!token_is_valid(token)) {
      fprintf(rsp, "%"PRIu32"\n", g_bar_manager.padding_right ? g_bar_manager.padding_right : 0);
    } else {
      bar_manager_set_padding_right(&g_bar_manager, atoi(token_to_string(token)));
    }
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

#undef VIEW_SET_PROPERTY

struct selector
{
  struct token token;
  bool did_parse;

  union {
    int dir;
    uint32_t did;
    uint64_t sid;
    struct window *window;
  };
};

enum label_type
{
  LABEL_SPACE,
};

void handle_message(FILE *rsp, char *message)
{
  struct token domain = get_token(&message);
  if (token_equals(domain, DOMAIN_CONFIG)) {
    handle_domain_config(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_ADD)){
    handle_domain_add(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_REMOVE)){
    handle_domain_remove(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_SET)){
    handle_domain_set(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_UPDATE)) {
    bar_manager_script_update(&g_bar_manager, true);
  } else if (token_equals(domain, DOMAIN_PUSH)) {
    handle_domain_push(rsp, domain, message);
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

static SOCKET_DAEMON_HANDLER(message_handler)
{
  struct event *event = event_create_p1(&g_event_loop, DAEMON_MESSAGE, message, sockfd);
  event_loop_post(&g_event_loop, event);
}
