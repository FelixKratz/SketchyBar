---
id: components
title: Special Components
sidebar_position: 1
---
## Components -- Special Items with special properties
Components are essentially items, but with special properties.
Currently there are the components (more details in the corresponding sections below):
* *graph*: showing a graph,
* *space*: representing a mission control space
* *bracket*: brackets together other items
* *alias*: a default menu bar item

### Data Graph -- Draws an arbitrary graph into the bar
```bash
sketchybar --add graph <name> <position> <width in points>
```

Additional graph properties:

| <property\>        | <value\>     | default    | description                 |
| :-------:          | :------:     | :-------:  | -----------                 |
| `graph.color`      | `<argb_hex>` | `0xcccccc` | Color of the graph line     |
| `graph.fill_color` | `<argb_hex>` | `0xcccccc` | Fill color of the graph     |
| `graph.line_width` | `<float>`    | `0.5`      | Width of the line in points |

Push data points into the graph via:
```bash
sketchybar --push <name> <data point>
```
where the `data point` is a floating point number between 0 and 1.

### Space -- Associate mission control spaces with an item
```bash
sketchybar --add space <name> <position>
```
The space component overrides the definition of the following properties and they must be set to correctly associate a mission control space with this item:
* *associated_space*: Which space this item represents
* *associated_display*: On which display the *associated_space* is shown.
The space component has additional variables available in *scripts*:
```bash
$SELECTED
$SID
$DID
```
where *$SELECTED* has the value *true* if the associated space is selected and *false* if the selected space is not selected, while
`$SID` holds the space id and `$DID` the display id.

By default the space component invokes the following script:
```bash
sketchybar --set $NAME icon.highlight=$SELECTED
```
which you can freely configure to your liking by supplying a different script to the space component:
```bash
sketchybar --set <name> script=<script/path>
```

For performance reasons the space script is only run on change.

### Item Bracket -- Group Items in e.g. colored sections
It is possible to bracket together items via the command (see [this](https://github.com/FelixKratz/SketchyBar/discussions/12#discussioncomment-1455842) discussion for an example):
```bash
sketchybar --add bracket <name> <first item name> ... <n-th item name>
```
The first item must always be the one listed earliest in the config. It is now possible to
set properties for the bracket, just as for any item or component. Brackets currently only support all background features.
E.g., if I wanted a colored background around *all* my space components (which are named *code*, *writing*, *reading* and *entertainment*) I would set it up like this:
```bash
sketchybar --add bracket primary_spaces code                        \
                                        writing                     \
                                        reading                     \
                                        entertainment               \
                                                                    \
           --set         primary_spaces background.color=0xffffffff \
                                        background.corner_radius=4  \
                                        background.height=20
```
this draws a white background below all my space components. I plan to expand the capability of item brackets significantly in the future.

### Item Alias -- Mirror items of the original macOS status bar into sketchybar
It is possible to create an alias for default menu bar items (such as MeetingBar, etc.) in sketchybar. The default menu bar can be set to autohide and this should still work.

It is now possible to create an alias of a default menu bar item with the following syntax:
```bash
sketchybar --add alias <application_name> <position>
```
this operation requires *screen capture permissions*, which should be granted in the system preferences.
This will put the default item into sketchybar. 
Aliases currently are not clickable but can be modified with all the options available for simple items.

The command can be overloaded by providing a *window_owner* and a *window_name*
```bash
sketchybar --add alias <window_owner>,<window_name> <position>
```
this way the default system items can also be slurped into sketchybar, e.g.:
- "Control Center,Bluetooth"
- "Control Center,WiFi"

Or the individual widgets of [Stats](https://github.com/exelban/stats):
- "Stats,CPU_Mini"
- etc...

All further default menu items currently available on your system can be found via the command:
```bash
sketchybar --query default_menu_items
```
You can override the color of an alias via the property (HEAD only):
```bash
sketchybar --set <name> alias.color=<rgba_hex>
```

