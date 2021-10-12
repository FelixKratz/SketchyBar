#ifndef MESSAGE_H
#define MESSAGE_H

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
#define COMMAND_SUBSCRIBE_MOUSE_ENTERED                     "mouse_entered"
#define COMMAND_SUBSCRIBE_MOUSE_EXITED                      "mouse_exited"
#define COMMAND_SUBSCRIBE_MOUSE_CLICKED                     "mouse_clicked"

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
#define COMMAND_QUERY_DEFAULTS                              "defaults"
#define COMMAND_QUERY_BAR                                   "bar"

#define ARGUMENT_COMMON_VAL_ON                              "on"
#define ARGUMENT_COMMON_VAL_TRUE                            "true"
#define ARGUMENT_COMMON_VAL_YES                             "yes"
#define ARGUMENT_COMMON_VAL_OFF                             "off"
#define ARGUMENT_COMMON_VAL_FALSE                           "false"
#define ARGUMENT_COMMON_VAL_NO                              "no"
#define ARGUMENT_COMMON_VAL_TOGGLE                          "toggle"
#define ARGUMENT_COMMON_NO_SPACE                            "nospace" 

#define ARGUMENT_UPDATES_WHEN_SHOWN                         "when_shown" 

#define BAR_DISPLAY_MAIN_ONLY                               "main"
#define BAR_DISPLAY_ALL                                     "all"

#define BAR_POSITION_BOTTOM                                 "bottom"
#define BAR_POSITION_TOP                                    "top"


struct token {
    char *text;
    unsigned int length;
};

static SOCKET_DAEMON_HANDLER(message_handler);
void handle_message(int sockfd, char *message);

#endif
