#include "message.h"

extern struct event_loop g_event_loop;
extern struct display_manager g_display_manager;
extern struct space_manager g_space_manager;
extern struct window_manager g_window_manager;
extern struct mouse_state g_mouse_state;
extern struct bar_manager g_bar_manager;
extern bool g_verbose;

#define DOMAIN_CONFIG  "config"

/* --------------------------------DOMAIN CONFIG-------------------------------- */
#define COMMAND_CONFIG_DEBUG_OUTPUT            "debug_output"
#define COMMAND_CONFIG_BAR_TEXT_FONT           "text_font"
#define COMMAND_CONFIG_BAR_ICON_FONT           "icon_font"
#define COMMAND_CONFIG_BAR_SPACE_ICON_COLOR    "space_icon_color"
#define COMMAND_CONFIG_BAR_BATTERY_ICON_COLOR  "battery_icon_color"
#define COMMAND_CONFIG_BAR_POWER_ICON_COLOR    "power_icon_color"
#define COMMAND_CONFIG_BAR_CLOCK_ICON_COLOR    "clock_icon_color"
#define COMMAND_CONFIG_BAR_DND_ICON_COLOR      "dnd_icon_color"
#define COMMAND_CONFIG_BAR_BACKGROUND          "background_color"
#define COMMAND_CONFIG_BAR_FOREGROUND          "foreground_color"
#define COMMAND_CONFIG_BAR_SPACE_STRIP         "space_icon_strip"
#define COMMAND_CONFIG_BAR_POWER_STRIP         "power_icon_strip"
#define COMMAND_CONFIG_BAR_SPACE_ICON          "space_icon"
#define COMMAND_CONFIG_BAR_CLOCK_ICON          "clock_icon"
#define COMMAND_CONFIG_BAR_CLOCK_FORMAT        "clock_format"
#define COMMAND_CONFIG_BAR_DND_ICON            "dnd_icon"
#define COMMAND_CONFIG_BAR_POSITION            "position"
#define COMMAND_CONFIG_BAR_HEIGHT              "height"
#define COMMAND_CONFIG_BAR_SPACING_LEFT        "spacing_left"
#define COMMAND_CONFIG_BAR_SPACING_RIGHT       "spacing_right"

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
    } else if (token_equals(command, COMMAND_CONFIG_BAR_POWER_STRIP)) {
        char **icon_strip = NULL;
        struct token token = get_token(&message);
        while (token.text && token.length > 0) {
            buf_push(icon_strip, token_to_string(token));
            token = get_token(&message);
        }
        bar_manager_set_power_strip(&g_bar_manager, icon_strip);
        if (buf_len(g_bar_manager._power_icon_strip) != 2) {
            daemon_fail(rsp, "value for '%.*s' must contain exactly two symbols separated by whitespace.\n", command.length, command.text);
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACE_ICON)) {
        struct token token = get_token(&message);
        if (!token_is_valid(token)) {
            fprintf(rsp, "%s\n", g_bar_manager._space_icon ? g_bar_manager._space_icon : "");
        } else {
            bar_manager_set_space_icon(&g_bar_manager, token_to_string(token));
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_CLOCK_ICON)) {
        struct token token = get_token(&message);
        if (!token_is_valid(token)) {
            fprintf(rsp, "%s\n", g_bar_manager._clock_icon ? g_bar_manager._clock_icon : "");
        } else {
            bar_manager_set_clock_icon(&g_bar_manager, token_to_string(token));
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_CLOCK_FORMAT)) {
        struct token token = get_token(&message);
        if (!token_is_valid(token)) {
            fprintf(rsp, "%s\n", g_bar_manager._clock_icon ? g_bar_manager._clock_icon : "");
        } else {
            bar_manager_set_clock_format(&g_bar_manager, token_to_string(token));
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
    } else if (token_equals(command, COMMAND_CONFIG_BAR_BATTERY_ICON_COLOR)) {
      struct token value = get_token(&message);
      if (!token_is_valid(value)) {
	fprintf(rsp, "0x%x\n", g_bar_manager.battery_icon_color.p);
      } else {
	uint32_t color = token_to_uint32t(value);
	if (color) {
	  bar_manager_set_battery_icon_color(&g_bar_manager, color);
	} else {
	  daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
	}
      }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_POWER_ICON_COLOR)) {
      struct token value = get_token(&message);
      if (!token_is_valid(value)) {
	fprintf(rsp, "0x%x\n", g_bar_manager.power_icon_color.p);
      } else {
	uint32_t color = token_to_uint32t(value);
	if (color) {
	  bar_manager_set_power_icon_color(&g_bar_manager, color);
	} else {
	  daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
	}
      }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_CLOCK_ICON_COLOR)) {
      struct token value = get_token(&message);
      if (!token_is_valid(value)) {
	fprintf(rsp, "0x%x\n", g_bar_manager.clock_icon_color.p);
      } else {
	uint32_t color = token_to_uint32t(value);
	if (color) {
	  bar_manager_set_clock_icon_color(&g_bar_manager, color);
	} else {
	  daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
	}
      }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_DND_ICON_COLOR)) {
      struct token value = get_token(&message);
      if (!token_is_valid(value)) {
	fprintf(rsp, "0x%x\n", g_bar_manager.dnd_icon_color.p);
      } else {
	uint32_t color = token_to_uint32t(value);
	if (color) {
	  bar_manager_set_dnd_icon_color(&g_bar_manager, color);
	} else {
	  daemon_fail(rsp, "unknown value '%.*s' given to command '%.*s' for domain '%.*s'\n", value.length, value.text, command.length, command.text, domain.length, domain.text);
	}
      }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_DND_ICON)) {
      struct token token = get_token(&message);
      if (!token_is_valid(token)) {
	fprintf(rsp, "%s\n", g_bar_manager._dnd_icon ? g_bar_manager._dnd_icon : "");
      } else {
	bar_manager_set_dnd_icon(&g_bar_manager, token_to_string(token));
      }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_POSITION)) {
      int length = strlen(message);
      char * top = "top";
      char * bottom = "bottom";
      if (length <= 0) {
	fprintf(rsp, "%s\n", g_bar_manager.position);
      } else if ((strcmp(message,top) == 0) || (strcmp(message,bottom) == 0)) {
	bar_manager_set_position(&g_bar_manager, string_copy(message));
      } else {
	daemon_fail(rsp, "value for '%.*s' must be either 'top' or 'bottom'.\n", command.length, command.text);
      }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_HEIGHT)) {
        struct token token = get_token(&message);
        if (!token_is_valid(token)) {
	  fprintf(rsp, "%"PRIu32"\n", g_bar_manager.height ? g_bar_manager.height : 0);
        } else {
	  bar_manager_set_height(&g_bar_manager, atoi(token_to_string(token)));
        }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACING_LEFT)) {
      struct token token = get_token(&message);
      if (!token_is_valid(token)) {
	fprintf(rsp, "%"PRIu32"\n", g_bar_manager.spacing_left ? g_bar_manager.spacing_left : 0);
      } else {
	bar_manager_set_spacing_left(&g_bar_manager, atoi(token_to_string(token)));
      }
    } else if (token_equals(command, COMMAND_CONFIG_BAR_SPACING_RIGHT)) {
      struct token token = get_token(&message);
      if (!token_is_valid(token)) {
	fprintf(rsp, "%"PRIu32"\n", g_bar_manager.spacing_right ? g_bar_manager.spacing_right : 0);
      } else {
	bar_manager_set_spacing_right(&g_bar_manager, atoi(token_to_string(token)));
      }
    } else {
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
    } else {
        daemon_fail(rsp, "unknown domain '%.*s'\n", domain.length, domain.text);
    }
}

static SOCKET_DAEMON_HANDLER(message_handler)
{
    struct event *event = event_create_p1(&g_event_loop, DAEMON_MESSAGE, message, sockfd);
    event_loop_post(&g_event_loop, event);
}
