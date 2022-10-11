#pragma once

#define DOMAIN_ADD                                          "--add"
#define COMMAND_ADD_ITEM                                    "item"                                  
#define COMMAND_ADD_COMPONENT                               "component"
#define COMMAND_ADD_EVENT                                   "event"

#define DOMAIN_UPDATE                                       "--update"

#define DOMAIN_PUSH                                         "--push"

#define DOMAIN_TRIGGER                                      "--trigger"

#define DOMAIN_DEFAULT                                      "--default"
#define COMMAND_DEFAULT_RESET                               "reset"

#define DOMAIN_CLONE                                        "--clone"

#define DOMAIN_RENAME                                       "--rename"

#define DOMAIN_REORDER                                      "--reorder"

#define DOMAIN_REMOVE                                       "--remove"

#define DOMAIN_MOVE                                         "--move"

#define DOMAIN_SET                                          "--set"

#define DOMAIN_ANIMATE                                      "--animate"

#define DOMAIN_EXIT                                         "--exit"

#define SUB_DOMAIN_ICON                                     "icon"
#define SUB_DOMAIN_LABEL                                    "label"
#define SUB_DOMAIN_BACKGROUND                               "background"
#define SUB_DOMAIN_GRAPH                                    "graph"
#define SUB_DOMAIN_ALIAS                                    "alias"
#define SUB_DOMAIN_POPUP                                    "popup"
#define SUB_DOMAIN_SHADOW                                   "shadow"
#define SUB_DOMAIN_IMAGE                                    "image"

#define PROPERTY_FONT                                       "font"
#define PROPERTY_COLOR                                      "color"
#define PROPERTY_HIGHLIGHT                                  "highlight"
#define PROPERTY_HIGHLIGHT_COLOR                            "highlight_color"
#define PROPERTY_PADDING_LEFT                               "padding_left"
#define PROPERTY_PADDING_RIGHT                              "padding_right"
#define PROPERTY_HEIGHT                                     "height"
#define PROPERTY_BORDER_COLOR                               "border_color"
#define PROPERTY_BORDER_WIDTH                               "border_width"
#define PROPERTY_CORNER_RADIUS                              "corner_radius"
#define PROPERTY_FILL_COLOR                                 "fill_color"
#define PROPERTY_LINE_WIDTH                                 "line_width"
#define PROPERTY_BLUR_RADIUS                                "blur_radius"
#define PROPERTY_DRAWING                                    "drawing"
#define PROPERTY_DISTANCE                                   "distance"
#define PROPERTY_ANGLE                                      "angle"
#define PROPERTY_SCALE                                      "scale"

#define PROPERTY_UPDATES                                    "updates"
#define PROPERTY_POSITION                                   "position"
#define PROPERTY_ASSOCIATED_DISPLAY                         "associated_display"
#define PROPERTY_ASSOCIATED_SPACE                           "associated_space"
#define PROPERTY_UPDATE_FREQ                                "update_freq"
#define PROPERTY_SCRIPT                                     "script"
#define PROPERTY_CLICK_SCRIPT                               "click_script"
#define PROPERTY_ICON                                       "icon"
#define PROPERTY_YOFFSET                                    "y_offset"
#define PROPERTY_WIDTH                                      "width"
#define PROPERTY_LABEL                                      "label"
#define PROPERTY_CACHE_SCRIPTS                              "cache_scripts"
#define PROPERTY_LAZY                                       "lazy"
#define PROPERTY_IGNORE_ASSOCIATION                         "ignore_association"
#define PROPERTY_EVENT_PORT                                 "mach_helper"

#define DOMAIN_BAR                                          "--bar"
#define PROPERTY_POSITION                                   "position"
#define PROPERTY_MARGIN                                     "margin"
#define PROPERTY_DISPLAY                                    "display"
#define PROPERTY_TOPMOST                                    "topmost"
#define PROPERTY_STICKY                                     "sticky"
#define PROPERTY_HIDDEN                                     "hidden"
#define PROPERTY_FONT_SMOOTHING                             "font_smoothing"
#define PROPERTY_SHADOW                                     "shadow"
#define PROPERTY_ALIGN                                      "align"
#define PROPERTY_NOTCH_WIDTH                                "notch_width"
#define PROPERTY_NOTCH_OFFSET                               "notch_offset"
#define PROPERTY_HORIZONTAL                                 "horizontal"

#define DOMAIN_SUBSCRIBE                                    "--subscribe"
#define COMMAND_SUBSCRIBE_FRONT_APP_SWITCHED                "front_app_switched"
#define COMMAND_SUBSCRIBE_SPACE_CHANGE                      "space_change"
#define COMMAND_SUBSCRIBE_DISPLAY_CHANGE                    "display_change"
#define COMMAND_SUBSCRIBE_SYSTEM_WOKE                       "system_woke"
#define COMMAND_SUBSCRIBE_SYSTEM_WILL_SLEEP                 "system_will_sleep"
#define COMMAND_SUBSCRIBE_VOLUME_CHANGE                     "volume_change"
#define COMMAND_SUBSCRIBE_MOUSE_ENTERED                     "mouse.entered"
#define COMMAND_SUBSCRIBE_MOUSE_EXITED                      "mouse.exited"
#define COMMAND_SUBSCRIBE_MOUSE_CLICKED                     "mouse.clicked"
#define COMMAND_SUBSCRIBE_MOUSE_ENTERED_GLOBAL              "mouse.entered.global"
#define COMMAND_SUBSCRIBE_MOUSE_EXITED_GLOBAL               "mouse.exited.global"

#define DOMAIN_QUERY                                        "--query"
#define COMMAND_QUERY_DEFAULT_ITEMS                         "default_menu_items"
#define COMMAND_QUERY_ITEM                                  "item"
#define COMMAND_QUERY_DEFAULTS                              "defaults"
#define COMMAND_QUERY_BAR                                   "bar"
#define COMMAND_QUERY_EVENTS                                "events"

#define ARGUMENT_COMMON_VAL_ON                              "on"
#define ARGUMENT_COMMON_VAL_NOT_OFF                         "!off"
#define ARGUMENT_COMMON_VAL_TRUE                            "true"
#define ARGUMENT_COMMON_VAL_NOT_FALSE                       "!false"
#define ARGUMENT_COMMON_VAL_ONE                             "1"
#define ARGUMENT_COMMON_VAL_NOT_ZERO                        "!0"
#define ARGUMENT_COMMON_VAL_YES                             "yes"
#define ARGUMENT_COMMON_VAL_NOT_NO                          "!no"
#define ARGUMENT_COMMON_VAL_OFF                             "off"
#define ARGUMENT_COMMON_VAL_NOT_ON                          "!on"
#define ARGUMENT_COMMON_VAL_FALSE                           "false"
#define ARGUMENT_COMMON_VAL_NOT_TRUE                        "!true"
#define ARGUMENT_COMMON_VAL_ZERO                            "0"
#define ARGUMENT_COMMON_VAL_NOT_ONE                         "!1"
#define ARGUMENT_COMMON_VAL_NO                              "no"
#define ARGUMENT_COMMON_VAL_NOT_YES                         "!yes"
#define ARGUMENT_COMMON_VAL_TOGGLE                          "toggle"
#define ARGUMENT_COMMON_VAL_BEFORE                          "before"
#define ARGUMENT_COMMON_VAL_AFTER                           "after"

#define ARGUMENT_UPDATES_WHEN_SHOWN                         "when_shown" 
#define ARGUMENT_DYNAMIC                                    "dynamic"

#define POSITION_TOP 't'
#define POSITION_BOTTOM 'b'
#define POSITION_LEFT   'l'
#define POSITION_RIGHT  'r'
#define POSITION_CENTER 'c'
#define POSITION_POPUP 'p'
#define POSITION_CENTER_LEFT 'q'
#define POSITION_CENTER_RIGHT 'e'

#define DISPLAY_MAIN 'm'
#define DISPLAY_ALL 'a'


#define BAR_ITEM            'i'
#define BAR_COMPONENT_GRAPH 'g'
#define BAR_COMPONENT_SPACE 's'
#define BAR_COMPONENT_ALIAS 'a'
#define BAR_COMPONENT_GROUP 'b'
#define BAR_PLUGIN          'p'

#define REGEX_DELIMITER     '/'
