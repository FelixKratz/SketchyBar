#include "message.h"

extern struct event_loop g_event_loop;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern struct bar_manager g_bar_manager;
extern bool g_verbose;

//TODO: Add signal setup for bar_item to listen on events and update dynamically
//TODO: Make builtin components for things we can not achive with external scripting

// TODO: Rewrite these horrible interfaces
#define DOMAIN_ADD "add"
// Syntax: spacebar -m add item <name> <position>
#define COMMAND_ADD_ITEM                                    "item"                                  
// Syntax: spacebar -m add component <name> <position>
#define COMMAND_ADD_COMPONENT                               "component"
// Syntax: spacebar -m add plugin <name> <position>
#define COMMAND_ADD_PLUGIN                                  "plugin"

#define DOMAIN_SET "set"

/* --------------------------------DOMAIN SET-------------------------------- */

// Syntax: spacebar -m set <name> <property> <value>
#define COMMAND_SET_UPDATE_FREQ                             "update_freq"
#define COMMAND_SET_SCRIPT                                  "script"
#define COMMAND_SET_PADDING_LEFT                            "padding_left"
#define COMMAND_SET_PADDING_RIGHT                           "padding_right"
#define COMMAND_SET_ICON                                    "icon"
#define COMMAND_SET_ICON_FONT                               "icon_font"
#define COMMAND_SET_ICON_COLOR                              "icon_color"

#define COMMAND_SET_LABEL                                   "label"
#define COMMAND_SET_LABEL_COLOR                             "label_color"
#define COMMAND_SET_LABEL_FONT                              "label_font"

#define DOMAIN_CONFIG  "config"

/* --------------------------------DOMAIN CONFIG-------------------------------- */
#define COMMAND_CONFIG_DEBUG_OUTPUT                        "debug_output"
#define COMMAND_CONFIG_BAR_TEXT_FONT                       "text_font"
#define COMMAND_CONFIG_BAR_ICON_FONT                       "icon_font"
#define COMMAND_CONFIG_BAR_SPACE_ICON_COLOR                "space_icon_color"
#define COMMAND_CONFIG_BAR_SPACE_ICON_COLOR_SECONDARY      "space_icon_color_secondary"
#define COMMAND_CONFIG_BAR_SPACE_ICON_COLOR_TERTIARY       "space_icon_color_tertiary"
#define COMMAND_CONFIG_BAR_BACKGROUND                      "background_color"
#define COMMAND_CONFIG_BAR_FOREGROUND                      "foreground_color"
#define COMMAND_CONFIG_BAR_SPACE_STRIP                     "space_icon_strip"
#define COMMAND_CONFIG_BAR_SPACE_ICON                      "space_icon"
#define COMMAND_CONFIG_BAR_POSITION                        "position"
#define COMMAND_CONFIG_BAR_HEIGHT                          "height"
#define COMMAND_CONFIG_BAR_PADDING_LEFT                    "padding_left"
#define COMMAND_CONFIG_BAR_PADDING_RIGHT                   "padding_right"
#define COMMAND_CONFIG_BAR_TITLE                           "title"
#define COMMAND_CONFIG_BAR_SPACES                          "spaces"
#define COMMAND_CONFIG_BAR_SPACES_FOR_ALL_DISPLAYS         "spaces_for_all_displays"
#define COMMAND_CONFIG_BAR_DISPLAY_SEPARATOR               "display_separator"
#define COMMAND_CONFIG_BAR_DISPLAY_SEPARATOR_ICON          "display_separator_icon"
#define COMMAND_CONFIG_BAR_DISPLAY_SEPARATOR_ICON_COLOR    "display_separator_icon_color"
#define COMMAND_CONFIG_BAR_DISPLAY                         "display"

/* --------------------------------COMMON ARGUMENTS----------------------------- */
#define ARGUMENT_COMMON_VAL_ON     "on"
#define ARGUMENT_COMMON_VAL_OFF    "off"

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

static uint32_t token_to_uint32t(struct token token)
{
  uint32_t result = 0;
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  sscanf(buffer, "%x", &result);
  return result;
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

// Syntax: spacebar -m add <item> <name>
static void handle_domain_add(FILE* rsp, struct token domain, char* message) {
  printf("Handling add domain \n");
  struct token command  = get_token(&message);
  if (token_equals(command, COMMAND_ADD_ITEM)) {
    struct token name = get_token(&message);
    struct token position = get_token(&message);
    printf("Reallocing \n");
    g_bar_manager.bar_items = (struct bar_item**) realloc(g_bar_manager.bar_items, sizeof(struct bar_item*) * (g_bar_manager.bar_item_count + 1));
    g_bar_manager.bar_item_count += 1;
    g_bar_manager.bar_items[g_bar_manager.bar_item_count - 1] = bar_item_create();
    printf("Realloced, now we init \n");
    bar_item_init(g_bar_manager.bar_items[g_bar_manager.bar_item_count - 1]);
    g_bar_manager.bar_items[g_bar_manager.bar_item_count - 1]->position = token_to_string(position)[0];
    bar_item_set_name(g_bar_manager.bar_items[g_bar_manager.bar_item_count - 1], string_copy(token_to_string(name)));
    printf("Init done, now we roll \n");
  }
}

// Syntax: spacebar -m set <name> <property> <value>
static void handle_domain_set(FILE* rsp, struct token domain, char* message) {
  struct token name  = get_token(&message);
  struct token property = get_token(&message);

  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, token_to_string(name));
  if (item_index_for_name < 0) {
    printf("Name not found in bar items");
    return;
  }

  if (token_equals(property, COMMAND_SET_ICON)) {
    bar_item_set_icon(g_bar_manager.bar_items[item_index_for_name], string_copy(message));
  } else if (token_equals(property, COMMAND_SET_ICON_FONT)) {
    bar_item_set_icon_font(g_bar_manager.bar_items[item_index_for_name], string_copy(message));
  } else if (token_equals(property, COMMAND_SET_LABEL)) {
    bar_item_set_label(g_bar_manager.bar_items[item_index_for_name], string_copy(message));
  } else if (token_equals(property, COMMAND_SET_LABEL_FONT)) {
    bar_item_set_label_font(g_bar_manager.bar_items[item_index_for_name], string_copy(message));
  } else if (token_equals(property, COMMAND_SET_SCRIPT)) {
    bar_item_set_script(g_bar_manager.bar_items[item_index_for_name], string_copy(message));
  } else if (token_equals(property, COMMAND_SET_UPDATE_FREQ)) {
    struct token value = get_token(&message);
    g_bar_manager.bar_items[item_index_for_name]->update_frequency = token_to_uint32t(value);
  } else if (token_equals(property, COMMAND_SET_LABEL_COLOR)) {
    struct token value = get_token(&message);
    bar_item_set_label_color(g_bar_manager.bar_items[item_index_for_name], token_to_uint32t(value));
  } else if (token_equals(property, COMMAND_SET_ICON_COLOR)) {
    struct token value = get_token(&message);
    bar_item_set_icon_color(g_bar_manager.bar_items[item_index_for_name], token_to_uint32t(value));
  }

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
  } else if (token_equals(command, COMMAND_CONFIG_BAR_TEXT_FONT)) {
    int length = strlen(message);
    if (length <= 0) {
      fprintf(rsp, "%s\n", g_bar_manager.t_font_prop);
    } else {
      bar_manager_set_text_font(&g_bar_manager, string_copy(message));
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_ICON_FONT)) {
    int length = strlen(message);
    if (length <= 0) {
      fprintf(rsp, "%s\n", g_bar_manager.i_font_prop);
    } else {
      bar_manager_set_icon_font(&g_bar_manager, string_copy(message));
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
  } else if (token_equals(command, COMMAND_CONFIG_BAR_FOREGROUND)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "0x%x\n", g_bar_manager.foreground_color.p);
    } else {
      uint32_t color = token_to_uint32t(value);
      if (color) {
        bar_manager_set_foreground_color(&g_bar_manager, color);
      } else {
        daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
      }
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_STRIP)) {
    char **icon_strip = NULL;
    struct token token = get_token(&message);

    if(token.length==0){ // If no value given, print current value
      fprintf(rsp, "\"%s\"", *g_bar_manager._space_icon_strip);
      char **p = g_bar_manager._space_icon_strip;
      for(int i = 1; i<buf_len(g_bar_manager.space_icon_strip);i++) {
        fprintf(rsp, " \"%s\"", *++p);
      }
    } else { // Else, set value
      while (token.text && token.length > 0) {
        buf_push(icon_strip, token_to_string(token));
        token = get_token(&message);
      }
      bar_manager_set_space_strip(&g_bar_manager, icon_strip);
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_ICON)) {
    struct token token = get_token(&message);
    if (!token_is_valid(token)) {
      fprintf(rsp, "%s\n", g_bar_manager._space_icon ? g_bar_manager._space_icon : "");
    } else {
      bar_manager_set_space_icon(&g_bar_manager, token_to_string(token));
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_ICON_COLOR)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "0x%x\n", g_bar_manager.space_icon_color.p);
    } else {
      uint32_t color = token_to_uint32t(value);
      if (color) {
        bar_manager_set_space_icon_color(&g_bar_manager, color);
      } else {
        daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
      }
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_ICON_COLOR_SECONDARY)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "0x%x\n", g_bar_manager.space_icon_color_secondary.p);
    } else {
      uint32_t color = token_to_uint32t(value);
      if (color) {
        bar_manager_set_space_icon_color_secondary(&g_bar_manager, color);
      } else {
        daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
      }
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_ICON_COLOR_TERTIARY)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "0x%x\n", g_bar_manager.space_icon_color_tertiary.p);
    } else {
      uint32_t color = token_to_uint32t(value);
      if (color) {
        bar_manager_set_space_icon_color_tertiary(&g_bar_manager, color);
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
  } else if (token_equals(command, COMMAND_CONFIG_BAR_TITLE)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "%s\n", bool_str[g_bar_manager.title]);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
      daemon_fail(rsp, "'%.*s' cannot be 'on' when 'center_shell' is 'on'.\n", command.length, command.text);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
      bar_manager_set_title(&g_bar_manager, false);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
      bar_manager_set_title(&g_bar_manager, true);
    } else {
      daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACES)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "%s\n", bool_str[g_bar_manager.spaces]);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
      bar_manager_set_spaces(&g_bar_manager, false);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
      bar_manager_set_spaces(&g_bar_manager, true);
    } else {
      daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACES_FOR_ALL_DISPLAYS)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "%s\n", bool_str[g_bar_manager.spaces_for_all_displays]);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
      bar_manager_set_spaces_for_all_displays(&g_bar_manager, false);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
      bar_manager_set_spaces_for_all_displays(&g_bar_manager, true);
    } else {
      daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_DISPLAY_SEPARATOR)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "%s\n", bool_str[g_bar_manager.display_separator]);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_OFF)) {
      bar_manager_set_display_separator(&g_bar_manager, false);
    } else if (token_equals(value, ARGUMENT_COMMON_VAL_ON)) {
      bar_manager_set_display_separator(&g_bar_manager, true);
    } else {
      daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_DISPLAY_SEPARATOR_ICON_COLOR)) {
    struct token value = get_token(&message);
    if (!token_is_valid(value)) {
      fprintf(rsp, "0x%x\n", g_bar_manager.display_separator_icon_color.p);
    } else {
      uint32_t color = token_to_uint32t(value);
      if (color) {
        bar_manager_set_display_separator_icon_color(&g_bar_manager, color);
      } else {
        daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
      }
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_DISPLAY_SEPARATOR_ICON)) {
    struct token token = get_token(&message);
    if (!token_is_valid(token)) {
      fprintf(rsp, "%s\n", g_bar_manager._display_separator_icon ? g_bar_manager._display_separator_icon : "");
    } else {
      bar_manager_set_display_separator_icon(&g_bar_manager, token_to_string(token));
    }
  } else if (token_equals(command, COMMAND_CONFIG_BAR_DISPLAY)) {
    int length = strlen(message);
    char * main = "main";
    char * all = "all";
    if (length <= 0) {
      fprintf(rsp, "%s\n", g_bar_manager.display);
    } else if ((strcmp(message,main) == 0) || (strcmp(message,all) == 0)) {
      bar_manager_set_display(&g_bar_manager, string_copy(message));
    } else {
      daemon_fail(rsp, "value for '%.*s' must be either 'main' or 'all'.\n", command.length, command.text);
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
  printf("Handling Message \n");
  struct token domain = get_token(&message);
  if (token_equals(domain, DOMAIN_CONFIG)) {
    printf("DOMAIN: CONFIG \n");
    handle_domain_config(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_ADD)){
    printf("DOMAIN: ADD \n");
    handle_domain_add(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_SET)){
    printf("DOMAIN: SET \n");
    handle_domain_set(rsp, domain, message);
  } else {
    daemon_fail(rsp, "unknown domain '%.*s'\n", domain.length, domain.text);
  }
}

static SOCKET_DAEMON_HANDLER(message_handler)
{
  struct event *event = event_create_p1(&g_event_loop, DAEMON_MESSAGE, message, sockfd);
  event_loop_post(&g_event_loop, event);
}
