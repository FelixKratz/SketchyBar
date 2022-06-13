#include "message.h"
#include "misc/defines.h"

extern struct event_loop g_event_loop;
extern struct bar_manager g_bar_manager;
extern bool g_verbose;

static void handle_domain_subscribe(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);

  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                name.text    );
  if (item_index_for_name < 0) {
    respond(rsp, "[!] Subscribe: Item not found '%s'\n", name.text);
    return;
  }
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];

  bar_item_parse_subscribe_message(bar_item, message, rsp);
}

static void handle_domain_trigger(FILE* rsp, struct token domain, char* message) {
  struct token event = get_token(&message);
  struct env_vars env_vars;
  env_vars_init(&env_vars);
  struct token token = get_token(&message);
  while (token.text && token.length > 0) {
    struct key_value_pair key_value_pair = get_key_value_pair(token.text, '=');
    
    if (key_value_pair.key && key_value_pair.value) {
      env_vars_set(&env_vars,
                   string_copy(key_value_pair.key),
                   string_copy(key_value_pair.value));
    }

    token = get_token(&message);
  }
  bar_manager_custom_events_trigger(&g_bar_manager, event.text, &env_vars);
  env_vars_destroy(&env_vars);
}

static void handle_domain_push(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token y = get_token(&message);
  
  int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                name.text    );

  if (item_index_for_name < 0) {
    respond(rsp, "[!] Push: Item '%s' not found", name.text);
    return;
  }
  struct bar_item* bar_item = g_bar_manager.bar_items[item_index_for_name];
  graph_push_back(&bar_item->graph, token_to_float(y));
  bar_item_needs_update(bar_item);
}

static void handle_domain_rename(FILE* rsp, struct token domain, char* message) {
  struct token old_name  = get_token(&message);
  struct token new_name  = get_token(&message);
  int item_index_for_old_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                    old_name.text);
  int item_index_for_new_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                    new_name.text);
  if (item_index_for_old_name < 0 || item_index_for_new_name >= 0) {
    respond(rsp, "[!] Rename: Failed to rename item: %s -> %s\n", old_name.text,
                                                                  new_name.text);
    return;
  }
  bar_item_set_name(g_bar_manager.bar_items[item_index_for_old_name],
                    token_to_string(new_name)                        );
}

static void handle_domain_clone(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token parent = get_token(&message);
  struct token modifier = get_token(&message);
  struct bar_item* parent_item = NULL;

  int parent_index = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                         parent.text    );

  if (parent_index >= 0)
    parent_item = g_bar_manager.bar_items[parent_index];
  else {
    respond(rsp, "[!] Clone: Parent Item '%s' not found\n", parent.text);
    return;
  }

  if (bar_manager_get_item_index_for_name(&g_bar_manager, name.text) >= 0) {
    respond(rsp, "[?] Clone: Item '%s' already exists\n", name.text);
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

static void handle_domain_add(FILE* rsp, struct token domain, char* message) {
  struct token command  = get_token(&message);

  if (token_equals(command, COMMAND_ADD_EVENT)) {
    struct token event = get_token(&message);
    if (strlen(message) > 0)
      custom_events_append(&g_bar_manager.custom_events,
                           token_to_string(event),
                           token_to_string(get_token(&message)));
    else custom_events_append(&g_bar_manager.custom_events,
                              token_to_string(event),
                              NULL                         );
    return;
  } 

  struct token name = get_token(&message);
  struct token position = get_token(&message);

  if (bar_manager_get_item_index_for_name(&g_bar_manager, name.text) >= 0) {
    respond(rsp, "[?] Add: Item '%s' already exists\n", name.text);
    return;
  } 
  struct bar_item* bar_item = bar_manager_create_item(&g_bar_manager);

  bar_item_set_type(bar_item, command.text[0]);
  bar_item_set_position(bar_item, position.text[0]);
  bar_item_set_name(bar_item, token_to_string(name));

  if (token_equals(command, COMMAND_ADD_ITEM)) {
  } else if (command.length > 0) {
    if (bar_item->type == BAR_COMPONENT_GRAPH) {
      struct token width = get_token(&message);
      graph_setup(&bar_item->graph, token_to_uint32t(width));
    }
    else if (bar_item->type == BAR_COMPONENT_ALIAS) {
      char* tmp_name = string_copy(name.text);
      struct key_value_pair key_value_pair = get_key_value_pair(tmp_name, ',');
      if (!key_value_pair.key || !key_value_pair.value)
        alias_setup(&bar_item->alias, token_to_string(name), NULL);
      else
        alias_setup(&bar_item->alias,
                    string_copy(key_value_pair.key),
                    string_copy(key_value_pair.value));

      free(tmp_name);
    }
    else if (bar_item->type == BAR_COMPONENT_GROUP) {
      struct token member = position;

      int index = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                      member.text    );

      if (index > 0
          && g_bar_manager.bar_items[index]->position == POSITION_POPUP) {

        popup_add_item(&g_bar_manager.bar_items[index]->parent->popup,
                       bar_item                                       );
        bar_item->position = POSITION_POPUP;
      }
      if (index >= 0)
        group_add_member(bar_item->group, g_bar_manager.bar_items[index]);
      else {
        respond(rsp, "[?] Add (Group) %s: Failed to add member '%s', item not found\n", bar_item->name, member.text);
      }
      member = get_token(&message);
      while (member.text && member.length > 0) {
        
        int index = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                        member.text    );
        if (index >= 0)
          group_add_member(bar_item->group, g_bar_manager.bar_items[index]);
        else {
          respond(rsp, "[?] Add (Group) %s: Failed to add member '%s', item not found\n", bar_item->name, member.text);
        }
        member = get_token(&message);
      }
    }
  } else {
    respond(rsp, "[!] Add %s: Invalid item type '%s'\n", bar_item->name, command.text);
    return;
  }

  if (position.text[0] == POSITION_POPUP) {
    char* pair = string_copy(position.text);
    struct key_value_pair key_value_pair = get_key_value_pair(pair, '.');
    if (key_value_pair.key && key_value_pair.value) {
      int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                    key_value_pair.value);
      if (item_index_for_name < 0) {
        respond(rsp,
                "[!] Add (Popup) %s: Item '%s' is not a valid popup host\n",
                bar_item->name,
                key_value_pair.value,
                bar_item->name                                                          );

        free(pair);
        bar_item_set_position(bar_item, POSITION_LEFT);
        return;
      }
      struct bar_item* target_item = g_bar_manager.bar_items[item_index_for_name];
      popup_add_item(&target_item->popup, bar_item);
    }
    free(pair);
  }

  bar_item_needs_update(bar_item);
}

static void handle_domain_default(FILE* rsp, struct token domain, char* message) {
  bar_item_parse_set_message(&g_bar_manager.default_item, message, rsp);
}

static bool handle_domain_bar(FILE *rsp, struct token domain, char *message) {
  struct token command  = get_token(&message);
  bool needs_refresh = false;

  if (token_equals(command, PROPERTY_MARGIN)) {
    struct token token = get_token(&message);
    ANIMATE(bar_manager_set_margin,
            &g_bar_manager,
            g_bar_manager.margin,
            token_to_int(token)    );

  } else if (token_equals(command, PROPERTY_YOFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(bar_manager_set_y_offset,
            &g_bar_manager,
            g_bar_manager.y_offset,
            token_to_int(token)      );

  } else if (token_equals(command, PROPERTY_BLUR_RADIUS)) {
    struct token token = get_token(&message);
    ANIMATE(bar_manager_set_background_blur,
            &g_bar_manager,
            g_bar_manager.blur_radius,
            token_to_int(token)             );

  } else if (token_equals(command, PROPERTY_FONT_SMOOTHING)) {
    struct token state = get_token(&message);
    needs_refresh = bar_manager_set_font_smoothing(&g_bar_manager,
                                                   evaluate_boolean_state(state,
                                                                          g_bar_manager.font_smoothing));
  } else if (token_equals(command, PROPERTY_SHADOW)) {
    struct token state = get_token(&message);
    needs_refresh = bar_manager_set_shadow(&g_bar_manager,
                                           evaluate_boolean_state(state,
                                                                  g_bar_manager.shadow));
  } else if (token_equals(command, PROPERTY_NOTCH_WIDTH)) {
    struct token token = get_token(&message);
    ANIMATE(bar_manager_set_notch_width,
            &g_bar_manager,
            g_bar_manager.notch_width,
            token_to_int(token)         );

  } else if (token_equals(command, PROPERTY_NOTCH_OFFSET)) {
    struct token token = get_token(&message);
    ANIMATE(bar_manager_set_notch_offset,
            &g_bar_manager,
            g_bar_manager.notch_offset,
            token_to_int(token)         );

  } else if (token_equals(command, PROPERTY_HIDDEN)) {
    struct token state = get_token(&message);
    uint32_t adid = 0;
    if (token_equals(state, "current")) {
      adid = display_arrangement(display_active_display_id());

      if (adid > 0 && adid <= g_bar_manager.bar_count)
        needs_refresh = bar_manager_set_hidden(&g_bar_manager,
                                               adid,
                                               !g_bar_manager.bars[adid - 1]->hidden);
      else
        printf("No bar on display %u \n", adid);
    } else needs_refresh = bar_manager_set_hidden(&g_bar_manager,
                                                  adid,
                                                  evaluate_boolean_state(state,
                                                                         g_bar_manager.any_bar_hidden));
  } else if (token_equals(command, PROPERTY_TOPMOST)) {
    struct token token = get_token(&message);
    needs_refresh = bar_manager_set_topmost(&g_bar_manager,
                                            evaluate_boolean_state(token,
                                                                   g_bar_manager.topmost));
  } else if (token_equals(command, PROPERTY_DISPLAY)) {
    struct token position = get_token(&message);
    if (position.length > 0)
      needs_refresh = bar_manager_set_display(&g_bar_manager,
                                              position.text[0]);
  } else if (token_equals(command, PROPERTY_POSITION)) {
    struct token position = get_token(&message);
    if (position.length > 0)
      needs_refresh = bar_manager_set_position(&g_bar_manager, position.text[0]);
  }
  else
    needs_refresh = background_parse_sub_domain(&g_bar_manager.background, rsp, command, message);

  return needs_refresh;
}

static char* reformat_batch_key_value_pair(struct token token) {
  struct key_value_pair key_value_pair = get_key_value_pair(token.text, '=');
  if (!key_value_pair.key) return NULL;
  char* rbr_msg = malloc((strlen(key_value_pair.key) + (key_value_pair.value
                                                        ? strlen(key_value_pair.value)
                                                        : 0)
                          + 3) * sizeof(char)                                         ); 

  pack_key_value_pair(rbr_msg, &key_value_pair);
  return rbr_msg;
}

static char* get_batch_line(char** message) {
  char* cursor = *message;
  bool end_of_batch = false;
  while (true) {
    if (*cursor == '\0' && *(cursor + 1) == '\0') {
      end_of_batch = true;
      break;
    }

    if (*cursor == '\0' && *(cursor + 1) == '-')
      break;

    cursor++;
  }
  char* rbr_msg = malloc(sizeof(char) * (cursor - *message + 2));
  memcpy(rbr_msg, *message, sizeof(char) * (cursor - *message + 1));
  *(rbr_msg + (cursor - *message + 1)) = '\0';
  if (end_of_batch) *message = cursor;
  else *message = cursor + 1;
  return rbr_msg;
}

static void handle_domain_query(FILE* rsp, struct token domain, char* message) {
  struct token token = get_token(&message);

  if (token_equals(token, COMMAND_QUERY_DEFAULT_ITEMS)) {
    print_all_menu_items(rsp);
  } else if (token_equals(token, COMMAND_QUERY_ITEM)) {
    struct token name  = get_token(&message);
    int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                  name.text      );
    if (item_index_for_name < 0) {
      respond(rsp, "[!] Query: Item '%s' not found\n", name.text);
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
    int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                  name.text      );
    if (item_index_for_name < 0) {
      respond(rsp, "[!] Query: Invalid query, or item '%s' not found \n", name.text);
      return;
    }
    bar_item_serialize(g_bar_manager.bar_items[item_index_for_name], rsp);
  }
}

static void handle_domain_remove(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  uint32_t count = 0;
  struct bar_item** bar_items = NULL;

  if (name.length > 1 && name.text[0] == REGEX_DELIMITER
      && name.text[name.length - 1] == REGEX_DELIMITER  ) {
    char* regstring = malloc(sizeof(char)*(name.length - 1));
    memcpy(regstring, &name.text[1], name.length - 2);
    regstring[name.length - 2] = '\0';
    regex_t regex;
    int reti;
    reti = regcomp(&regex, regstring, 0);
    free(regstring);
    if (reti) {
      respond(rsp, "[!] Remove: Could not compile regex '%s'\n", name.text);
      return;
    }

    for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
      struct bar_item* bar_item = g_bar_manager.bar_items[i];

      reti = regexec(&regex, bar_item->name, 0, NULL, 0);
      if (!reti) {
        count++;
        bar_items = realloc(bar_items, sizeof(struct bar_item*)*count);
        bar_items[count - 1] = bar_item;
      }
      else if (reti != REG_NOMATCH) {
        char buf[100];
        regerror(reti, &regex, buf, sizeof(buf));
        respond(rsp, "[!] Remove: Regex match failed '%s'\n", buf);
        return;
      }
    }
    regfree(&regex);
  }
  else {
    int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager,
                                                                  name.text      );
    if (item_index_for_name < 0) {
      respond(rsp, "[!] Remove: Item '%s' not found\n", name.text);
      return;
    }
    bar_items = realloc(bar_items, sizeof(struct bar_item*));
    bar_items[0] = g_bar_manager.bar_items[item_index_for_name];
    count = 1;
  }
  if (!bar_items || count == 0) return;

  for (int i = 0; i < count; i++) {
    bar_manager_remove_item(&g_bar_manager, bar_items[i]);
  }

  if (bar_items) free(bar_items);
}

static void handle_domain_move(FILE* rsp, struct token domain, char* message) {
  struct token name = get_token(&message);
  struct token direction = get_token(&message);
  struct token reference = get_token(&message);

  int item_index = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
  int reference_item_index = bar_manager_get_item_index_for_name(&g_bar_manager, reference.text);
  if (item_index < 0 || reference_item_index < 0) {
      respond(rsp, "[!] Move: Item '%s' or '%s' not found\n", name.text, reference.text);
      return;
  }

  bar_manager_move_item(&g_bar_manager,
                        g_bar_manager.bar_items[item_index],
                        g_bar_manager.bar_items[reference_item_index],
                        token_equals(direction, ARGUMENT_COMMON_VAL_BEFORE));

  bar_item_needs_update(g_bar_manager.bar_items[item_index]);
}

static void handle_domain_order(FILE* rsp, struct token domain, char* message) {
  struct bar_item* ordering[g_bar_manager.bar_item_count];
  memset(ordering, 0, sizeof(struct bar_item*)*g_bar_manager.bar_item_count);

  uint32_t count = 0;
  struct token name = get_token(&message);
  while (name.text && name.length > 0) {
    int index = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
    if (index < 0) {
      respond(rsp, "[!] Order: Item '%s' not found\n", name.text);
      name = get_token(&message);
      continue;
    }
    ordering[count] = g_bar_manager.bar_items[index];
    count++;

    name = get_token(&message);
  }

  bar_manager_sort(&g_bar_manager, ordering, count);
  bar_manager_freeze(&g_bar_manager);
  g_bar_manager.frozen = false;
  bar_manager_refresh(&g_bar_manager, false);
  bar_manager_unfreeze(&g_bar_manager);
}

void handle_message_mach(struct mach_buffer* buffer) {
  if (!buffer->message.descriptor.address) return;
  char* message = buffer->message.descriptor.address;
  char* response = NULL;
  size_t length = 0;
  FILE* rsp = open_memstream(&response, &length);
  fprintf(rsp, "");

  g_bar_manager.animator.interp_function = '\0';
  g_bar_manager.animator.duration = 0;
  bar_manager_freeze(&g_bar_manager);
  struct token command = get_token(&message);
  bool bar_needs_refresh = false;

  while (command.text && command.length > 0) {
    if (token_equals(command, DOMAIN_SET)) {
      struct token name = get_token(&message);
      uint32_t count = 0;
      struct bar_item** bar_items = NULL;

      if (name.length > 1 && name.text[0] == REGEX_DELIMITER
          && name.text[name.length - 1] == REGEX_DELIMITER  ) {
        char* regstring = malloc(sizeof(char)*(name.length - 1));
        memcpy(regstring, &name.text[1], name.length - 2);
        regstring[name.length - 2] = '\0';
        regex_t regex;
        int reti;
        reti = regcomp(&regex, regstring, 0);
        free(regstring);
        if (reti) {
          respond(rsp, "[!] Set: Could not compile regex '%s'\n", name.text);
          break;
        }

        for (int i = 0; i < g_bar_manager.bar_item_count; i++) {
          struct bar_item* bar_item = g_bar_manager.bar_items[i];

          reti = regexec(&regex, bar_item->name, 0, NULL, 0);
          if (!reti) {
            count++;
            bar_items = realloc(bar_items, sizeof(struct bar_item*)*count);
            bar_items[count - 1] = bar_item;
          }
          else if (reti != REG_NOMATCH) {
            char buf[1024];
            regerror(reti, &regex, buf, sizeof(char)*1024);
            respond(rsp, "[?] Set: No match found for regex '%s'\n", name.text);
            break;
          }
        }
        regfree(&regex);
      }
      else {
        int item_index_for_name = bar_manager_get_item_index_for_name(&g_bar_manager, name.text);
        if (item_index_for_name < 0) {
          respond(rsp, "[!] Set: Item not found '%s'\n", name.text);
          break;
        }
        bar_items = realloc(bar_items, sizeof(struct bar_item*));
        bar_items[0] = g_bar_manager.bar_items[item_index_for_name];
        count = 1;
      }
      if (!bar_items || count == 0) return;

      struct token token = get_token(&message);
      while (token.text && token.length > 0) {
        for (int i = 0; i < count; i++) {
          struct token tmp = {string_copy(token.text), token.length};
          char* rbr_msg = reformat_batch_key_value_pair(tmp);
          free(tmp.text);
          if (!rbr_msg) {
            respond(rsp, "[!] Set (%s): Expected <key>=<value> pair, but got: '%s'\n", bar_items[i]->name, token.text);
            break;
          }
          bar_item_parse_set_message(bar_items[i], rbr_msg, rsp);
          free(rbr_msg);
        }
        if (message && message[0] == '-') break;
        token = get_token(&message);
      }
      free(bar_items);
    } else if (token_equals(command, DOMAIN_DEFAULT)) {
      struct token token = get_token(&message);
      while (token.text && token.length > 0) {
        char* rbr_msg = reformat_batch_key_value_pair(token);
          if (!rbr_msg) {
            respond(rsp, "[!] Set (default): Expected <key>=<value> pair, but got: '%s'\n", token.text);
            break;
          }
        handle_domain_default(rsp, command, rbr_msg);
        free(rbr_msg);
        if (message && message[0] == '-') break;
        token = get_token(&message);
      }
    } else if (token_equals(command, DOMAIN_ANIMATE)) {
      g_bar_manager.animator.interp_function = get_token(&message).text[0];
      g_bar_manager.animator.duration = token_to_uint32t(get_token(&message));
    } else if (token_equals(command, DOMAIN_BAR)) {
      struct token token = get_token(&message);
      while (token.text && token.length > 0) {
        char* rbr_msg = reformat_batch_key_value_pair(token);
        if (!rbr_msg) {
          respond(rsp, "[!] Bar: Expected <key>=<value> pair, but got: '%s'\n", token.text);
          break;
        }
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
    } else if (token_equals(command, DOMAIN_EXIT)) {
      bar_manager_destroy(&g_bar_manager);
      exit(0);
    } else {
      char* rbr_msg = get_batch_line(&message);
      respond(rsp, "[!] Unknown domain '%s'\n", command.text);
      free(rbr_msg);
    }
    command = get_token(&message);
  }

  if (bar_needs_refresh) {
    bar_manager_resize(&g_bar_manager);
    g_bar_manager.bar_needs_update = true;
  }

  g_bar_manager.frozen = false;
  bar_manager_refresh(&g_bar_manager, false);
  bar_manager_unfreeze(&g_bar_manager);

  if (rsp) fclose(rsp);

  mach_send_message(buffer->message.header.msgh_remote_port, response,
                                                             length,
                                                             false    );
  if (response) free(response);
}

MACH_HANDLER(mach_message_handler) {
  struct event *event = event_create(&g_event_loop, MACH_MESSAGE, message);
  event_loop_post(&g_event_loop, event);
}
