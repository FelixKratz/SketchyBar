sketchybar-components(5)

# NAME

sketchybar-components - configuration of special components in the bar

# SYNOPSIS

*sketchybar --add graph* _NAME_ _POSITION_ _WIDTH_ *--push* _NAME_
_DATA_POINT_ ... _DATA_POINT_ ++
*sketchybar --add space* _NAME_ _POSITION_ ++
*sketchybar --add bracket* _NAME_ _MEMBER_NAME_ ... _MEMBER_NAME_ ++
*sketchybar --add alias* _APPLICATION_NAME_|"_WINDOW_OWNER_,_WINDOW_NAME_" _POSITION_ ++
*sketchybar --add slider* _NAME_ _POSITION_ _WIDTH_

# DESCRIPTION

Components are essentially items (see *sketchybar-items*(5)), but with special
properties.

Currently, the available components include
- *graph*: shows a graph
- *space*: represents a mission control space
- *bracket*: creates a shared background for any number of items
- *alias*: aliases a menu bar item from the macOS bar
- *slider*: shows a progression and can be clicked/dragged to set a new value.
Refer to *sketchybar-types*(5) when needed.

# OPTIONS

The usual commands for items such as *--add*, *--set*, *--move*, etc. (see
*sketchybar-items*(5)) can also be used by components as is unless superseded by
a redefinition below.

# DATA GRAPH

## Options

*--add graph* _NAME_ _POSITION_ _WIDTH_
	Add an item to the bar that draws a graph. Inherits the usual *--add* command
	usage. _WIDTH_ is the width of the graph in point units.

*--set* _NAME_ _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_
	Change the properties of the component. Inherits the usual *--set* command
	usage. Additional properties include:

[[ *property*
:[ *value*
:[ *default*
:< *description*
|  graph.color
:  <argb_hex>
:  0xffcccccc
:  Color of the graph line
|  graph.fill_color
:  <argb_hex>
:  0xffcccccc
:  Fill color of the graph
|  graph.line_width
:  <float>
:  0.5
:  Width of the line in points


*--push* _NAME_ _DATA_POINT_ ... _DATA_POINT_
	Push data points to the graph. _DATA_POINT_ is a float between 0 and 1.

## Notes

Graphs usually take the entire height of the bar as a drawing canvas. However,
if a background is set for the graph item, the graph will draw inside of the
background (i.e. scaled in height to fit the background). With a background
enabled, the graph can also be moved via a *y_offset*, say,
```
sketchybar --set graph_name background.color=0xff00ff00 \\
					   background.height=20 \\
					   y_offset=2
```


# SPACE

## Options

*--add space* _NAME_ _POSITION_
	Add an item to the bar that associates with a mission control space. Inherits
	the usual *--add* command usage.

*--set* _NAME_ _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_
	Change the properties of the component. Inherits the usual *--set* command
	usage. Overriden properties include:
	- *space*: which space this item represents
	- *display*: (optional) on which display the space is shown.

	The space property must be set to properly associate this item with the
	corresponding mission control space. Optionally, a display to force a space
	item to stay on a specific display can be provided. Otherwise the item will
	draw on the screen on which the space is currently located.

## Notes

This component provides additional variables available for *script* and
*click_script*:
- *$SELECTED*: has the value *true* if the associated space is selected;
  otherwise *false*
- *$SID*: holds the space id
- *$DID*: holds the display id.

By default, this component invokes the script
```
sketchybar --set $NAME icon.highlight=$SELECTED
```
which can be freely configured by supplying a different script:
```
sketchybar --set space_name script=path/to/script
```

For performance reasons the space script is only run on a change in the
*$SELECTED* variable, i.e. if the associated space has become active or has
resigned being active.


# ITEM BRACKET

## Options

*--add bracket* _NAME_ _MEMBER_NAME_ ... _MEMBER_NAME_
	Add an item to the bar that groups other items together. Inherits the usual
	*--add* command usage. _MEMBER_NAME_ is a name of any item in the bar. It can
	also be a regular expression: /*REGEX*/.

*--set* _NAME_ _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_
	Change the properties of the graph. Currently *DOES NOT* inherit *all* of the
	usual *--set* command usage. Inherited properties include background-related
	properties only.


## Notes

It is now possible to set properties for the bracket, just as for any item or
component, although support is only for background as of yet. 

To set a colored background around the space components (which are, say, named
space.1, space.2, space.3), the setup would be:
```
sketchybar --add bracket spaces space.1 space.2 space.3 \\
		 --set spaces background.color=0xffffffff \\
				    background.corner_radius=4 \\
				    background.height=20
```

Alternatively, if there are a number of spaces, say, named space.1, space.2,
etc., the regex syntax comes in handy:
```
sketchybar --add bracket spaces '/space\..*/'
```

Brackets are very flexible with their members, i.e. it is no problem to bracket
a left and a center item together. The background will span all the way between
those items.

# ITEM ALIAS

## Options

*--add alias* _APPLICATION_NAME_ _POSITION_ ++
*--add alias* "_WINDOW_OWNER_,_WINDOW_NAME_" _POSITION_

	Add an item to the bar that mirrors items in the original macOS status bar.
	Inherits the usual *--add* command usage. _APPLICATION_NAME_ is the name of
	the application (e.g. MeetingBar, "Google Drive", etc.) that manages the menu
	bar item to be aliased.

	If an application has multiple menu bar widgets, the command can be overloaded
	by providing a _WINDOW_OWNER_ and _WINDOW_NAME_. See the note below on how to
	list all available widgets.

	Then, say, the default system items can be aliased as in:
	- "Control Center,Bluetooth"
	- "Control Center,WiFi"
	- ...

	Or, say, the individual widgets of Stats (see https://github.com/exelban/stats):
	- "Stats,CPU_Mini"
	- ...

*--set* _NAME_ _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_
	Change the properties of the graph. Inherits the usual *--set* command usage.
	Additional properties include:

[[ *property*
:[ *value*
:[ *default*
:< *description*
|  alias.color
:  <argb_hex>
:  -
:  Overriding color to use with the alias
|  alias.scale
:  <float>
:  -
:  The scale factor that should be applied to the alias
|  alias.update_freq
:  <positive_integer>
:  1
:  Time in seconds before the alias is updated


## Notes

All macOS menu bar items currently available on the system with their respective
owners and names can be listed via (see *sketchybar-query*(5)):
```
sketchybar --query default_menu_items
```


# SLIDER

## Options

*--add slider* _NAME_ _POSITION_ _WIDTH_
	Add an item to the bar that is a draggable progression indicator. Inherits
	the usual *--add* command usage. _WIDTH_ is the width of the graph in point
	units.

*--set* _NAME_ _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_
	Change the properties of the component. Inherits the usual *--set* command
	usage. Additional properties include:

[[ *property*
:[ *value*
:[ *default*
:< *description*
|  slider.width
:  <positive_integer>
:  100
:  Total width of the slider in points
|  slider.percentage
:  <positive_integer>
:  0
:  Progression of the slider in percent (0-100)
|  slider.highlight_color
:  <argb_hex>
:  0xff0000ff
:  Color that highlights the progression of the slider
|  slider.knob
:  <string>
:  -
:  Knob of the slider
|  slider.knob.<text_property>
:  -
:  -
:  The slider knob supports all text properties
|  slider.background.<background_property>
:  -
:  -
:  The slider supports all background properties

## Notes

The slider can be enabled to receive *mouse.clicked* events by subscribing to
this event. 

A slider will receive the additional environment variable *$PERCENTAGE* on a
click in its script, which represents the percentage corresponding to the click
location.

If a slider is dragged by the mouse it will only send a single event on drag
release and track the mouse during the drag.

# EXAMPLES

To add mission control space indicators to indicate active and available spaces
in bash:

```
SPACE_ICONS=("1" "2" "3" "4" "5" "6" "7" "8" "9" "10")
for i in "${!SPACE_ICONS[@]}"
do sid=$(($i+1))
	sketchybar --add space space.$sid left \\
		\--set space.$sid space=$sid icon=${SPACE_ICONS[i]} \\
		script='sketchybar --set $NAME icon.highlight=$SELECTED'
done
```
Should you wish to focus the respective spaces with a click on the item in the
bar, you can add a *click_script* to the items which triggers a space change
(e.g. via yabai, or via skhd by synthetically emitting the space focus keyboard
shortcuts).

# SEE ALSO

*sketchybar*(1)++
*sketchybar*(5)++
*sketchybar-items*(5)++
*sketchybar-popup*(5)++
*sketchybar-events*(5)++
*sketchybar-query*(5)++
*sketchybar-animate*(5)++
*sketchybar-types*(5)++
*sketchybar-tips*(5)
