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
* *alias*: an alias of a menu bar item from the macOS bar
* *slider*: a slider that shows a progression and can be clicked/dragged to set a new value

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
sketchybar --push <name> <data point> ... <data point>
```
where the `<data point>` is a floating point number between 0 and 1.

Graphs usually take the entire height of the bar as a drawing canvas, however,
if you set a background for the graph item and set a height for it, the graph
will draw inside of the background. With a background enabled, the graph can
also be moved via a `y_offset`, e.g.:
```bash
sketchybar --set <graph name> background.color=0xff00ff00 background.height=20 y_offset=2
```

### Space -- Associate mission control spaces with an item
```bash
sketchybar --add space <name> <position>
```
The space component overrides the definition of the following properties:
* *associated_space*: Which space this item represents
* (optional) *associated_display*: On which display the *associated_space* is shown.
The `associated_space` property must be set to properly associate this item with the corresponding mission control space.
Optionally, you can provide an `associated_display` to force a space item to stay on a specific display, otherwise the
item will draw on the screen on which the space is currently located. 

The space component has additional variables available in *scripts*:
```bash
$SELECTED
$SID
$DID
```
where `$SELECTED` has the value `true` if the associated space is selected and
`false` if the associated space is not selected, while
`$SID` holds the space id and `$DID` the display id.

By default the space component invokes the following script:
```bash
sketchybar --set $NAME icon.highlight=$SELECTED
```
which you can freely configure to your liking by supplying a different script
to the space component:
```bash
sketchybar --set <name> script=<script/path>
```

For performance reasons the space script is only run on a change in the
`$SELECTED` variable, i.e. if the associated space has become active
or has resigned being active.

### Item Bracket -- Group Items in e.g. colored sections
It is possible to create a common background for any number of items, i.e. to bracket together items, via the command:
```bash
sketchybar --add bracket <name> <member name> ... <member name>
```
The `<member name>` is a name of any item in the bar that should be added to the bracket.
The `<member name>` can also be a `/<regex>/` expression.
It is now possible to set properties for the bracket, just as for any item or component. Brackets currently only support all background features.
E.g., if I wanted a colored background around my space components (which are named *space.1*, *space.2*, *space.3*) I would set it up like this:
```bash
sketchybar --add bracket spaces space.1 space.2 space.3     \
           --set         spaces background.color=0xffffffff \
                                background.corner_radius=4  \
                                background.height=20
```
Alternatively, if I had a number of spaces, called *space.1*, *space.2*, etc. the regex syntax comes in handy:
```bash
sketchybar --add bracket spaces '/space\..*/'               \
           --set         spaces background.color=0xffffffff \
                                background.corner_radius=4  \
                                background.height=20
```
this draws a white background below all my space components.

Brackets are very flexible with their members, i.e. it is no problem to bracket together a `left` and a `center` item,
the background will span all the way between those items.

### Item Alias -- Mirror items of the original macOS status bar into sketchybar
It is possible to create an alias for default menu bar items
(such as MeetingBar, etc.) in sketchybar. The default menu bar can be set to
autohide and this should still work.

To create an alias of a default menu bar item use the following syntax:
```bash
sketchybar --add alias <application_name> <position>
```
this operation requires *screen capture permissions*, which should be granted
in the system preferences.

This will put the default macOS menu bar item into sketchybar. If an
application has multiple menu bar widgets the command can be overloaded by
providing a *window_owner* and a *window_name*
```bash
sketchybar --add alias "<window_owner>,<window_name>" <position>
```
this way the default system items can also be aliased in sketchybar as well,
e.g.:
- "Control Center,Bluetooth"
- "Control Center,WiFi"
- ...

Or the individual widgets of [Stats](https://github.com/exelban/stats):
- "Stats,CPU_Mini"
- etc...

All further macOS menu bar items currently available on your system can be
found via the command
```bash
sketchybar --query default_menu_items
```
where all items with their respective owner and name are listed.

You can override the color of an alias via the property:
```bash
sketchybar --set <name> alias.color=<argb_hex>
```

Aliases currently are not clickable but can be modified with all the options
available for simple items.

### Slider -- A draggable progression indicator
A slider can be added to the bar via the command:
```bash
sketchybar --add slider <name> <position> <width>
```
Like all components, the slider only adds some additional properties and
functionality to a regular item. Thus all properties of regular items are
available for the slider. Additionally the slider exposes the additional
properties:

| <property\>                    | <value\>             | default      | description                                         |
| :-------:                      | :------:             | :-------:    | -----------                                         |
| `slider.width`                 | `<positive_integer>` | `100`        | Total width of the slider in points                 |
| `slider.percentage`            | `<positive_integer>` | `0`          | Progression of the slider in percent (0-100)        |
| `slider.highlight_color`       | `<argb_hex>`         | `0xff0000ff` | Color that highlights the progression of the slider |
| `slider.knob`                  | `<string>`           |              | Knob of the slider                                  |
| `slider.knob.<text_property>`  |                      |              | The slider knob supports all `<text_property>`s     |
| `slider.<background_property>` |                      |              | The slider supports all `<background_property>`s    |

The slider can be enabled to receive `mouse.clicked` events by subscribing to this event.
A slider will receive the additional environment variable `$PERCENTAGE` on a click in its
script, which represents the percentage corresponding to the click location.
If a slider is dragged by the mouse it will only send a single event on drag release and
track the mouse during the drag.
