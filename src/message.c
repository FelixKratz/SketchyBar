#include "message.h"
#include "alias.h"
#include "background.h"
#include "bar_item.h"
#include "bar_manager.h"
#include "display.h"
#include "group.h"
#include "misc/helpers.h"
#include <_types/_uint32_t.h>
#include <string.h>

extern struct event_loop g_event_loop;
extern struct bar_manager g_bar_manager;
extern bool g_verbose;

static bool evaluate_boolean_state(struct token state, bool previous_state) {
  if (token_equals(state, ARGUMENT_COMMON_VAL_ON) || token_equals(state, ARGUMENT_COMMON_VAL_YES) || token_equals(state, ARGUMENT_COMMON_VAL_TRUE)) return true;
  else if (token_equals(state, ARGUMENT_COMMON_VAL_TOGGLE)) return !previous_state;
  else return false;
}

static void bar_item_parse_subscribe_message(struct bar_item* bar_item, char* message) {
  struct token event = get_token(&message);

  while (event.text && event.length > 0) {
    bar_item->update_mask |= custom_events_get_flag_for_name(&g_bar_manager.custom_events, event.text);
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
  graph_push_back(&bar_item->graph, token_to_float(y));
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
    if (strlen(message) > 0) custom_events_append(&g_bar_manager.custom_events, token_to_string(event), token_to_string(get_token(&message)));
    else custom_events_append(&g_bar_manager.custom_events, token_to_string(event), NULL);

    return;
  } 

  struct token name;
  struct token position;
  struct bar_item* bar_item = bar_manager_create_item(&g_bar_manager);

  if (token_equals(command, COMMAND_ADD_ITEM)) {
    name = get_token(&message);
    position = get_token(&message);
    bar_item->position = position.text[0];
    bar_item_set_type(bar_item, BAR_ITEM);
  } else if (token_equals(command, COMMAND_ADD_COMPONENT)) {
    struct token identifier = get_token(&message);
    name = get_token(&message);

    bar_item_set_type(bar_item, identifier.text[0]);
    if (bar_item->type == BAR_COMPONENT_GRAPH) {
      position = get_token(&message);
      bar_item->position = position.text[0];
      struct token width = get_token(&message);
      graph_init(&bar_item->graph, token_to_uint32t(width));
    }
    else if (bar_item->type == BAR_COMPONENT_SPACE) {
      position = get_token(&message);
      bar_item->position = position.text[0];
    }
    else if (bar_item->type == BAR_COMPONENT_ALIAS) {
      position = get_token(&message);
      bar_item->position = position.text[0];
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
    else if (bar_item->type == BAR_COMPONENT_GROUP) {
      struct token member = get_token(&message);
      while (member.text && member.length > 0) {
        int index = bar_manager_get_item_index_for_name(&g_bar_manager, member.text);
        if (index >= 0)
          group_add_member(bar_item->group, g_bar_manager.bar_items[index]);
        else {
          printf("Item %s not found! \n", member.text);
          fprintf(rsp, "Item %s not found! \n", member.text);
        }
        member = get_token(&message);
      }
    }
  } else {
    printf("Command: %s not found \n", command.text);
    fprintf(rsp, "Command: %s not found \n", command.text);
    return;
  }

  bar_item_set_name(bar_item, token_to_string(name));
  bar_manager_refresh(&g_bar_manager, true);
}

static void message_parse_set_message_for_bar_item(FILE* rsp, struct bar_item* bar_item, char* message) {
  bool needs_update = false;
  struct token property = get_token(&message);

  struct token subdom;
  struct token entry;
  get_key_value_pair(property.text, &subdom.text, &entry.text, '.');
  if (subdom.text && entry.text) {
    subdom.length = strlen(subdom.text);
    entry.length = strlen(entry.text);
    if (token_equals(subdom, SUB_DOMAIN_ICON))
      needs_update = text_parse_sub_domain(&bar_item->icon, rsp, entry, message);
    else if (token_equals(subdom, SUB_DOMAIN_LABEL))
      needs_update = text_parse_sub_domain(&bar_item->label, rsp, entry, message);
    else if (token_equals(subdom, SUB_DOMAIN_BACKGROUND))
      needs_update = background_parse_sub_domain(&bar_item->background, rsp, entry, message);
    else if (token_equals(subdom, SUB_DOMAIN_GRAPH))
      needs_update = graph_parse_sub_domain(&bar_item->graph, rsp, entry, message);
    else {
      fprintf(rsp, "Invalid subdomain: %s \n", subdom.text);
      printf("Invalid subdomain: %s \n", subdom.text);
    }
  }
  else if (token_equals(property, PROPERTY_ICON)) {
    needs_update = text_set_string(&bar_item->icon, token_to_string(get_token(&message)), false);
  } else if (token_equals(property, PROPERTY_LABEL)) {
    needs_update = text_set_string(&bar_item->label, token_to_string(get_token(&message)), false);
  } else if (token_equals(property, PROPERTY_UPDATES)) {
    struct token token = get_token(&message);
    if (token_equals(token, ARGUMENT_UPDATES_WHEN_SHOWN)) {
      bar_item->updates = true;
      bar_item->updates_only_when_shown = true;
    }
    else {
      bar_item->updates = evaluate_boolean_state(token, bar_item->updates);
      bar_item->updates_only_when_shown = false;
    }
  } else if (token_equals(property, PROPERTY_DRAWING)) {
    bar_item_set_drawing(bar_item, evaluate_boolean_state(get_token(&message), bar_item->drawing));
  } else if (token_equals(property, PROPERTY_WIDTH)) {
    struct token token = get_token(&message);
    if (token_equals(token, "dynamic"))
      bar_item->has_const_width = false;
    else {
      bar_item->has_const_width = true;
      bar_item->custom_width = token_to_uint32t(token);
    }
    needs_update = true;
  } else if (token_equals(property, PROPERTY_SCRIPT)) {
    bar_item_set_script(bar_item, string_copy(message));
  } else if (token_equals(property, PROPERTY_CLICK_SCRIPT)) {
    bar_item_set_click_script(bar_item, string_copy(message));
  } else if (token_equals(property, PROPERTY_UPDATE_FREQ)) {
    bar_item->update_frequency = token_to_uint32t(get_token(&message));
  } else if (token_equals(property, PROPERTY_POSITION)) {
    bar_item->position = get_token(&message).text[0];
  } else if (token_equals(property, PROPERTY_ASSOCIATED_SPACE)) {
    struct token token = get_token(&message);
    for (int i = 0; i < token.length; i++) {
      int sep = -1;
      if (token.text[i] == ',') token.text[i] = '\0', sep = i;
      bar_item_append_associated_space(bar_item, 1 << strtoul(&token.text[sep + 1], NULL, 0));
    }
  } else if (token_equals(property, PROPERTY_ASSOCIATED_DISPLAY)) {
    struct token token = get_token(&message);
    for (int i = 0; i < token.length; i++) {
      int sep = -1;
      if (token.text[i] == ',') token.text[i] = '\0', sep = i;
      bar_item_append_associated_display(bar_item, 1 << strtoul(&token.text[sep + 1], NULL, 0));
    }
  } else if (token_equals(property, PROPERTY_YOFFSET)) {
    bar_item_set_yoffset(bar_item, token_to_int(get_token(&message)));
  } else if (token_equals(property, PROPERTY_CACHE_SCRIPTS)) {
    bar_item->cache_scripts = evaluate_boolean_state(get_token(&message), bar_item->cache_scripts);
  } else if (token_equals(property, PROPERTY_LAZY)) {
    bar_item->lazy = evaluate_boolean_state(get_token(&message), bar_item->lazy);
  } else if (token_equals(property, COMMAND_DEFAULT_RESET)) {
    bar_item_init(&g_bar_manager.default_item, NULL);
  } else {
    fprintf(rsp, "Invalid propery: %s \n", property.text);
    printf("Invalid propery: %s \n", property.text);
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
  bool needs_refresh = false;


  if (token_equals(command, PROPERTY_MARGIN)) {
    struct token token = get_token(&message);
    g_bar_manager.margin = token_to_uint32t(token);
    needs_refresh = true;
  } else if (token_equals(command, PROPERTY_YOFFSET)) {
    struct token token = get_token(&message);
    g_bar_manager.y_offset = token_to_uint32t(token);
    needs_refresh = true;
  } else if (token_equals(command, PROPERTY_BLUR_RADIUS)) {
    struct token token = get_token(&message);
    bar_manager_set_background_blur(&g_bar_manager, token_to_uint32t(token));
  } else if (token_equals(command, PROPERTY_FONT_SMOOTHING)) {
    struct token state = get_token(&message);
    bar_manager_set_font_smoothing(&g_bar_manager, evaluate_boolean_state(state, g_bar_manager.font_smoothing));
  } else if (token_equals(command, PROPERTY_HIDDEN)) {
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
  } else if (token_equals(command, PROPERTY_TOPMOST)) {
    struct token token = get_token(&message);
    bar_manager_set_topmost(&g_bar_manager, evaluate_boolean_state(token, g_bar_manager.topmost));
  } else if (token_equals(command, PROPERTY_DISPLAY)) {
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
  } else if (token_equals(command, PROPERTY_POSITION)) {
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
  else
    needs_refresh = background_parse_sub_domain(&g_bar_manager.background, rsp, command, message);

  if (needs_refresh) {
    bar_manager_resize(&g_bar_manager);
    bar_manager_refresh(&g_bar_manager, true);
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
    } else if (token_equals(command, COMMAND_BATCH_BAR)) {
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
  } else if (token_equals(token, COMMAND_QUERY_DEFAULTS)) {
    bar_item_serialize(&g_bar_manager.default_item, rsp);
  }

}

void handle_message(int sockfd, char* message) {
  FILE* rsp = fdopen(sockfd, "w");

  struct token domain = get_token(&message);

  if (token_equals(domain, DOMAIN_SET)){
    handle_domain_set(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_BATCH)){
    handle_domain_batch(rsp, domain, message); 
  } else if (token_equals(domain, DOMAIN_PUSH)) {
    handle_domain_push(rsp, domain, message);
  } else if (token_equals(domain, DOMAIN_BAR)) {
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
  } else if (token_equals(domain, DOMAIN_QUERY)) {
    handle_domain_query(rsp, domain, message);
  } else {
    fprintf(rsp, "unknown domain '%.*s'\n", domain.length, domain.text);
    printf("unknown domain '%.*s'\n", domain.length, domain.text);
  }

  if (rsp) fclose(rsp);
}

static SOCKET_DAEMON_HANDLER(message_handler) {
  int* _sockfd = malloc(sizeof(int));
  memcpy(_sockfd, &sockfd, sizeof(int));
  struct event *event = event_create(&g_event_loop, DAEMON_MESSAGE, _sockfd);
  event_loop_post(&g_event_loop, event);
}
