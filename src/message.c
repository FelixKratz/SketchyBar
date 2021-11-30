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
  if (token_equals(state, ARGUMENT_COMMON_VAL_ON) || token_equals(state, ARGUMENT_COMMON_VAL_YES) || token_equals(state, ARGUMENT_COMMON_VAL_TRUE) || token_equals(state, ARGUMENT_COMMON_VAL_ONE)) return true;
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

// Syntax: sketchybar -m --subscribe <name> <event>
static void handle_domain_subscribe(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);

  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  if (item_index_for_name < 0) return;
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];

  bar_item_parse_subscribe_message(bar_item, message); 
}

// Syntax: sketchybar -m --trigger <event> 
static void handle_domain_trigger(FILE* rsp, struct token domain, char* message) {
  struct token event = get_token(&message);
  bar_manager_custom_events_trigger(&g_bar_manager, event.text);
}

// Syntax: sketchybar -m --push <name> <y>
static void handle_domain_push(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token y = get_token(&message);
  
  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  if (item_index_for_name < 0) return;
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];
  graph_push_back(&bar_item->graph, token_to_float(y));
  bar_item_needs_update(bar_item);
}

// Syntax sketchybar -m --rename <old name> <new name>
static void handle_domain_rename(FILE* rsp, struct token domain, char* message) {
  struct token old_name  = get_token(&message);
  struct token new_name  = get_token(&message);
  int item_index_for_old_name = bar_manager_get_item_index_for_name(&g_bar_manager, old_name.text);
  int item_index_for_new_name = bar_manager_get_item_index_for_name(&g_bar_manager, new_name.text);
  if (item_index_for_old_name < 0 || item_index_for_new_name >= 0) {
    fprintf(rsp, "Could not rename item: %s -> %s \n", old_name.text, new_name.text);
    printf("Could not rename item: %s -> %s \n", old_name.text, new_name.text);
    return;
  }
  bar_item_set_name(g_bar_manager.bar_items[item_index_for_old_name], token_to_string(new_name));
}

// Syntax: sketchybar -m --clone <name> <parent name>
static void handle_domain_clone(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token parent = get_token(&message);
  struct token modifier = get_token(&message);
  struct bar_item* parent_item = NULL;

  int parent_index = bar_manager_get_item_index_for_name(&g_bar_manager, parent.text);
  if (parent_index >= 0) parent_item = g_bar_manager.bar_items[parent_index];
  else {
    printf("Parent Item: %s does not exist \n", parent.text);
    fprintf(rsp, "Parent Item: %s does not exist \n", parent.text);
    return;
  }

  if (bar_manager_get_item_index_for_name(&g_bar_manager, name.text) >= 0) {
    fprintf(rsp, "Item %s already exists \n", name.text);
    return;
  }
  struct bar_item* bar_item = bar_manager_create_item(&g_bar_manager);
  bar_item_inherit_from_item(bar_item, parent_item);
  bar_item_set_name(bar_item, token_to_string(name));
  if (token_equals(modifier, ARGUMENT_COMMON_VAL_BEFORE))
    bar_manager_move_item(&g_bar_manager, bar_item, parent_item, true);
  else if (token_equals(modifier, ARGUMENT_COMMON_VAL_AFTER))
    bar_manager_move_item(&g_bar_manager, bar_item, parent_item, false);
  bar_item_needs_update(bar_item);
}

// Syntax: sketchybar -m --add <item|component identifer> <name>[:parent] [<position>]
static void handle_domain_add(FILE* rsp, struct token domain, char* message) {
  struct token command  = get_token(&message);

  if (token_equals(command, COMMAND_ADD_EVENT)) {
    struct token event = get_token(&message);
    if (strlen(message) > 0) custom_events_append(&g_bar_manager.custom_events, token_to_string(event), token_to_string(get_token(&message)));
    else custom_events_append(&g_bar_manager.custom_events, token_to_string(event), NULL);
    return;
  } 

  struct token name = get_token(&message);
  struct token position = get_token(&message);

  if (bar_manager_get_item_index_for_name(&g_bar_manager, name.text) >= 0) {
    fprintf(rsp, "Item %s already exists \n", name.text);
    return;
  } 
  struct bar_item* bar_item = bar_manager_create_item(&g_bar_manager);

  bar_item_set_type(bar_item, command.text[0]);
  bar_item->position = position.text[0];
  bar_item_set_name(bar_item, token_to_string(name));

  if (token_equals(command, COMMAND_ADD_ITEM)) {
  } else if (command.length > 0) {
    if (bar_item->type == BAR_COMPONENT_GRAPH) {
      struct token width = get_token(&message);
      graph_init(&bar_item->graph, token_to_uint32t(width));
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
    else if (bar_item->type == BAR_COMPONENT_GROUP) {
      struct token member = position;
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

  bar_item_needs_update(bar_item);
}

// Syntax: sketchybar -m --set <name> <property>=<value> ... <property>=<value>
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
    if (token_equals(token, ARGUMENT_DYNAMIC))
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
    uint32_t prev = bar_item->associated_space;
    bar_item->associated_space = 0;
    for (int i = 0; i < token.length; i++) {
      int sep = -1;
      if (token.text[i] == ',') token.text[i] = '\0', sep = i;
      bar_item_append_associated_space(bar_item, 1 << strtoul(&token.text[sep + 1], NULL, 0));
    }
    needs_update = prev != bar_item->associated_space;
  } else if (token_equals(property, PROPERTY_ASSOCIATED_DISPLAY)) {
    struct token token = get_token(&message);
    uint32_t prev = bar_item->associated_display;
    bar_item->associated_display = 0;
    for (int i = 0; i < token.length; i++) {
      int sep = -1;
      if (token.text[i] == ',') token.text[i] = '\0', sep = i;
      bar_item_append_associated_display(bar_item, 1 << strtoul(&token.text[sep + 1], NULL, 0));
    }
    needs_update = prev != bar_item->associated_display;
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

// Syntax: sketchybar -m --default <property>=<value> ... <property>=<value>
static void handle_domain_default(FILE* rsp, struct token domain, char* message) {
  message_parse_set_message_for_bar_item(rsp, &g_bar_manager.default_item, message);
}

// Syntax: sketchybar -m --bar <property>=<value> ... <property>=<value>
static bool handle_domain_bar(FILE *rsp, struct token domain, char *message) {
  struct token command  = get_token(&message);
  bool needs_refresh = false;

  if (token_equals(command, PROPERTY_MARGIN)) {
    struct token token = get_token(&message);
    g_bar_manager.margin = token_to_uint32t(token);
    needs_refresh = true;
  } else if (token_equals(command, PROPERTY_YOFFSET)) {
    struct token token = get_token(&message);
    g_bar_manager.y_offset = token_to_int(token);
    needs_refresh = true;
  } else if (token_equals(command, PROPERTY_BLUR_RADIUS)) {
    struct token token = get_token(&message);
    needs_refresh = bar_manager_set_background_blur(&g_bar_manager, token_to_uint32t(token));
  } else if (token_equals(command, PROPERTY_FONT_SMOOTHING)) {
    struct token state = get_token(&message);
    needs_refresh = bar_manager_set_font_smoothing(&g_bar_manager, evaluate_boolean_state(state, g_bar_manager.font_smoothing));
  } else if (token_equals(command, PROPERTY_SHADOW)) {
    struct token state = get_token(&message);
    needs_refresh = bar_manager_set_shadow(&g_bar_manager, evaluate_boolean_state(state, g_bar_manager.shadow));
  } else if (token_equals(command, PROPERTY_HIDDEN)) {
    struct token state = get_token(&message);
    struct token select = get_token(&message);
    uint32_t adid = 0;
    if (!(select.length == 0)) {
      adid = token_equals(select, "current") ? display_arrangement(display_active_display_id()) : atoi(select.text);
      if (adid > 0 && adid <= g_bar_manager.bar_count)
        needs_refresh = bar_manager_set_hidden(&g_bar_manager, adid, evaluate_boolean_state(state, g_bar_manager.bars[adid - 1]->hidden));
      else
        printf("No bar on display %u \n", adid);
    } else needs_refresh = bar_manager_set_hidden(&g_bar_manager, adid, evaluate_boolean_state(state, g_bar_manager.any_bar_hidden));
  } else if (token_equals(command, PROPERTY_TOPMOST)) {
    struct token token = get_token(&message);
    needs_refresh = bar_manager_set_topmost(&g_bar_manager, evaluate_boolean_state(token, g_bar_manager.topmost));
  } else if (token_equals(command, PROPERTY_DISPLAY)) {
    struct token position = get_token(&message);
    if (position.length > 0)
      needs_refresh = bar_manager_set_display(&g_bar_manager, position.text[0]);
    else {
      printf("value for %s must be either 'main' or 'all'.\n", command.text);
      fprintf(rsp, "value for %s must be either 'main' or 'all'.\n", command.text);
    }
  } else if (token_equals(command, PROPERTY_POSITION)) {
    struct token position = get_token(&message);
    if (position.length > 0)
      needs_refresh = bar_manager_set_position(&g_bar_manager, position.text[0]);
    else {
      printf("value for %s must be either 'top' or 'bottom'.\n", command.text);
      fprintf(rsp, "value for %s must be either 'top' or 'bottom'.\n", command.text);
    }
  }
  else
    needs_refresh = background_parse_sub_domain(&g_bar_manager.background, rsp, command, message);

  return needs_refresh;
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

// Syntax: sketchybar -m --query bar
// Syntax: sketchybar -m --query item <name>
// Syntax: sketchybar -m --query defaults
// Syntax: sketchybar -m --query events
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
  } else if (token_equals(token, COMMAND_QUERY_EVENTS)) {
    custom_events_serialize(&g_bar_manager.custom_events, rsp);
  } else {
    struct token name = token;
    int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
    if (item_index_for_name < 0) {
      fprintf(rsp, "Not a valid query, or item: %s not found \n", name.text);
      printf("Not a valid query, or item: %s not found \n", name.text);
      return;
    }
    bar_item_serialize(g_bar_manager.bar_items[item_index_for_name], rsp);
  }
}

// Syntax: sketchybar -m --remove <name>
static void handle_domain_remove(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  int item_index = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  if (item_index < 0) {
      fprintf(rsp, "Name: %s not found in bar items \n", name.text);
      printf("Name: %s not found in bar items \n", name.text);
      return;
  }
  bar_manager_remove_item(&g_bar_manager, g_bar_manager.bar_items[item_index]);
}

// Syntax: sketchybar -m --move <name> </> <reference>
static void handle_domain_move(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token direction = get_token(&message);
  struct token reference = get_token(&message);

  int item_index = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  int reference_item_index = bar_manager_get_item_index_for_name(&g_bar_manager, reference.text);
  if (item_index < 0 || reference_item_index < 0) {
      fprintf(rsp, "Name: %s or %s not found in bar items \n", name.text, reference.text);
      printf("Name: %s or %s not found in bar items \n", name.text, reference.text);
      return;
  }

  bar_manager_move_item(&g_bar_manager, g_bar_manager.bar_items[item_index], g_bar_manager.bar_items[reference_item_index], token_equals(direction, ARGUMENT_COMMON_VAL_BEFORE));
  bar_item_needs_update(g_bar_manager.bar_items[item_index]);
}

static void handle_domain_order(FILE* rsp, struct token domain, char* message) {
  struct bar_item* ordering[g_bar_manager.bar_item_count];
  memset(ordering, 0, sizeof(struct bar_item*)*g_bar_manager.bar_item_count);

  uint32_t count = 0;
  struct token name = get_token(&message);
  while (name.text && name.length > 0) {
    int index = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
    if (index < 0) continue;
    ordering[count] = g_bar_manager.bar_items[index];
    count++;

    name = get_token(&message);
  }

  bar_manager_sort(&g_bar_manager, ordering, count);
  bar_manager_refresh(&g_bar_manager, false);
}

void handle_message(int sockfd, char* message) {
  FILE* rsp = fdopen(sockfd, "w");

  bar_manager_freeze(&g_bar_manager);
  struct token command = get_token(&message);
  bool bar_needs_refresh = false;
  while (command.text && command.length > 0) {
    if (token_equals(command, DOMAIN_SET)) {
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
    } else if (token_equals(command, DOMAIN_DEFAULT)) {
      struct token token = get_token(&message);
      while (token.text && token.length > 0) {
        char* rbr_msg = reformat_batch_key_value_pair(token);
        if (!rbr_msg) break;
        handle_domain_default(rsp, command, rbr_msg);
        free(rbr_msg);
        if (message && message[0] == '-') break;
        token = get_token(&message);
      }
    } else if (token_equals(command, DOMAIN_BAR)) {
      struct token token = get_token(&message);
      while (token.text && token.length > 0) {
        char* rbr_msg = reformat_batch_key_value_pair(token);
        if (!rbr_msg) break;
        bar_needs_refresh |= handle_domain_bar(rsp, command, rbr_msg);
        free(rbr_msg);
        if (message && message[0] == '-') break;
        token = get_token(&message);
      }
    } else if (token_equals(command, DOMAIN_ADD)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_add(rsp, command, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_CLONE)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_clone(rsp, command, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_SUBSCRIBE)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_subscribe(rsp, command, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_PUSH)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_push(rsp, command, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_UPDATE)) {
      bar_manager_update_space_components(&g_bar_manager, true);
      bar_manager_update(&g_bar_manager, true);
      bar_needs_refresh = true;
    } else if (token_equals(command, DOMAIN_TRIGGER)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_trigger(rsp, command, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_QUERY)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_query(rsp, command, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_REORDER)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_order(rsp, command, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_MOVE)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_move(rsp, command, rbr_msg);
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_REMOVE)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_remove(rsp, command, rbr_msg);
      bar_needs_refresh = true;
      free(rbr_msg);
    } else if (token_equals(command, DOMAIN_RENAME)) {
      char* rbr_msg = get_batch_line(&message);
      handle_domain_rename(rsp, command, rbr_msg);
      free(rbr_msg);
    } else {
      char* rbr_msg = get_batch_line(&message);
      fprintf(rsp, "unknown domain %s\n", command.text);
      printf("unknown domain %s\n", command.text);
      free(rbr_msg);
    }
    command = get_token(&message);
  }
  bar_manager_unfreeze(&g_bar_manager);
  if (bar_needs_refresh) {
    bar_manager_resize(&g_bar_manager);
    bar_manager_refresh(&g_bar_manager, true);
  }
  else {
    bar_manager_refresh(&g_bar_manager, false);
  }
  if (rsp) fclose(rsp);
}

static SOCKET_DAEMON_HANDLER(message_handler) {
  int* _sockfd = malloc(sizeof(int));
  memcpy(_sockfd, &sockfd, sizeof(int));
  struct event *event = event_create(&g_event_loop, DAEMON_MESSAGE, _sockfd);
  event_loop_post(&g_event_loop, event);
}
