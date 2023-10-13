sketchybar-items(5)

# NAME

sketchybar-items - configuration of items in the bar

# SYNOPSIS

*sketchybar --add item* _NAME_ _POSITION_ ++
*sketchybar --set* _NAME_ _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_ ++
*sketchybar --default* _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_ ++
*sketchybar --reorder* _NAME_ ... _NAME_ ++
*sketchybar --move* _NAME_ *before*|*after* _REFERENCE_NAME_ ++
*sketchybar --clone* _PARENT_NAME_ _NAME_ [*before*|*after*] ++
*sketchybar --rename* _OLD_NAME_ _NEW_NAME_ ++
*sketchybar --remove* _NAME_

# DESCRIPTION

Items are the main building blocks of sketchybar and can be configured in a
number of ways. Items have the following basic structure:

```
  :                      :    -
--:----------------------:--  |
| : | icon | | | label | : |  | background.height
--:----------------------:--  |
 ^:^        ^ ^         ^:^   -
 | |        | |         | |________________________________
 | |        | |______________ label.padding_{left,right}  |
 | |_________________________ icon.padding_{left,right}   |
 |___________________________ background.padding_{left,right}

```

When neeeded, refer to *sketchybar-types*(5) for the nomenclature of types of
values found here.

# OPTIONS

*--add item* _NAME_ _POSITION_
	Add an item to the bar. _NAME_ should not contain whitespaces, or must be 
	quoted. It is later used to refer to this item in the configuration. _POSITION_ 
	is the placement in the bar and can be either *left*, *right*, *center*, or *q*
	(which is left of the notch) and *e* (which is right of the notch). The items
	will appear in the bar in the order in which they are added, but can be moved
	later on.

	For details about *q* and *e*, see
	https://github.com/FelixKratz/SketchyBar/issues/120

*--set* _NAME_ _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_
	Change the properties of an item. _NAME_ is used to target the item. It can
	be a regular expression inside of two slashes: /*REGEX*/ to target mutliple
	items. A list of properties available can be found in PROPERTIES. See below.
	
*--default* _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_
	Change the defaults at any point in the configuration. All item created
	*after* changing the defaults will inherit these properties from the default
	item.

*--reorder* _NAME_ ... _NAME_
	Reorder the items. A new order can be supplied for arbitrary many items. Only
	the specified items get reordered. By swapping them around, everything else
	stays the same. To swap two items, simply call
	```
	sketchybar --reorder NAME1 NAME2
	```

*--move* _NAME_ *before*|*after* _REFERENCE_NAME_
	Move items to specific positions. To move item _NAME_ before item
	_REFERENCE_NAME_, use *before*. Otherwise, use *after*.

*--clone* _PARENT_NAME_ _NAME_ [*before*|*after*]
	Clone an item to another item. The new item will inherit *all* properties of
	the parent item. The optional *before* and *after* modifiers can be used to
	move the item before, or after the parent, equivalently to a *--move* command.

*--rename* _OLD_NAME_ _NEW_NAME_
	Rename an item. Note that the new name should not be in use by another item.

*--remove* _NAME_
	Remove an item. _NAME_ can be a regex: /*REGEX*/. The item will be completely
	destroyed and removed from the bar, brackets and popups.

# PROPERTIES

This section is relevant for the *--set* and *--default* commands.

## Geometry properties

[[ *property*
:[ *value*
:[ *default*
:< *description*
|  drawing
:  <boolean>
:  on
:  If the item should be drawn into the bar
|  position
:  left, right, center
:  -
:  Position of the item in the bar
|  space
:  <positive_integer list>
:  0
:  Spaces to show this item on
|  display
:  <positive_integer list>, active
:  0
:  Displays to show this item on
|  ignore_association
:  <boolean>
:  off
:  Ignores all space / display associations while on
|  y_offset
:  <integer>
:  0
:  Vertical offset applied to the item
|  padding_left
:  <integer>
:  0
:  The padding applied left of the item
|  padding_right
:  <integer>
:  0
:  The padding applied right of the item
|  width
:  <positive_integer> or dynamic
:  dynamic
:  Makes the _item_ use a fixed _width_ given in points
|  blur_radius
:  <positive_integer>
:  0
:  The blur radius applied to the background of the item
|  background.<background_property>
:  -
:  -
:  Items support all background properties

## Icon properties

[[ *property*
:[ *value*
:[ *default*
:< *description*
|  icon
:  <string>
:  -
:  Icon of the item
|  icon.<text_property>
:  -
:  -
:  Icons support all _text_ properties

## Label properties

[[ *property*
:[ *value*
:[ *default*
:< *description*
|  label
:  <string>
:  -
:  Label of the item
|  label.<text_property>
:  -
:  -
:  Labels support all _text_ properties

## Scripting properties

[[ *property*
:[ *value*
:[ *default*
:< *description*
|  script
:  <path>, <string>
:  -
:  Script to run on an event
|  click_script
:  <path>, <string>
:  -
:  Script to run on a mouse click
|  update_freq
:  <positive_integer>
:  0
:  Time in seconds between routine script executions (0 means never)
|  updates
:  <boolean>, when_shown
:  on
:  If and when the item updates e.g. via script execution
|  mach_helper
:  <string>
:  -
:  Registers a helper for direct event notifications


Note:
- *click_script* is different to a *mouse.clicked* event. See
  https://github.com/FelixKratz/SketchyBar/discussions/109
- An example of *mach_helper* can be found at
  https://github.com/FelixKratz/SketchyBarHelper

## Text properties

[[ *text_property*
:[ *value*
:[ *default*
:< *description*
|  drawing
:  <boolean>
:  on
:  If the text is rendered
|  highlight
:  <boolean>
:  off
:  If the text uses the highlight_color or the regular color
|  color
:  <argb_hex>
:  0xffffffff
:  Color used to render the text
|  highlight_color
:  <argb_hex>
:  0xff000000
:  Highlight color of the text (e.g. for active space icon
|  padding_left
:  <integer>
:  0
:  Padding to the left of the text
|  padding_right
:  <integer>
:  0
:  Padding to the right of the text
|  y_offset
:  <integer>
:  0
:  Vertical offset applied to the text
|  font
:  <family>:<type>:<size>
:  Hack Nerd Font:Bold:14.0
:  The font to be used for the text
|  font.family
:  <string>
:  Hack Nerd Font
:  The font family to be used for the text
|  font.style
:  <string>
:  Bold
:  The font style to be used for the text
|  font.size
:  <float>
:  14.0
:  The font size to be used for the text
|  string
:  <string>
:  -
:  Sets the text to the specified string
|  width
:  <positive_integer> or dynamic
:  dynamic
:  Makes the text use a fixed width given in points
|  align
:  center, left, right
:  left
:  Aligns the text in its container when it has a fixed width larger than the content width
|  background.<background_property>
:  -
:  -
:  Texts support all background properties
|  shadow.<shadow_property>
:  -
:  -
:  Texts support all shadow properties

## Background properties

[[ *background_property*
:[ *value*
:[ *default*
:< *description*
|  drawing
:  <boolean>
:  off
:  If the background should be rendered
|  color
:  <argb_hex>
:  0x00000000
:  Fill color of the background
|  border_color
:  <argb_hex>
:  0x00000000
:  Color of the backgrounds border
|  border_width
:  <positive_integer>
:  0
:  Width of the background border
|  height
:  <positive_integer>
:  0
:  Overrides the height of the background
|  corner_radius
:  <positive_integer>
:  0
:  Corner radius of the background
|  padding_left
:  <integer>
:  0
:  Padding to the left of the background
|  padding_right
:  <integer>
:  0
:  Padding to the right of the background
|  y_offset
:  <integer>
:  0
:  Vertical offset applied to the background
|  clip
:  <float>
:  0.0
:  By how much the background clips the bar (i.e. transparent holes in the bar)
|  image
:  <path>, app.<bundle-id>
:  -
:  The path to a png or jpeg image file, or a bundle identifier of an application
|  image.<image_property>
:  -
:  -
:  Backgrounds support all image properties
|  shadow.<shadow_property>
:  -
:  -
:  Backgrounds support all shadow properties

## Image properties

[[ *image_property*
:[ *value*
:[ *default*
:< *description*
|  drawing
:  <boolean>
:  off
:  If the image should draw
|  scale
:  <float>
:  1.0
:  The scale factor that should be applied to the image

## Shadow properties

[[ *shadow_property*
:[ *value*
:[ *default*
:< *description*
|  drawing
:  <boolean>
:  off
:  If the shadow should be drawn
|  color
:  <argb_hex>
:  0xff000000
:  Color of the shadow
|  angle
:  <positive_integer>
:  30
:  Angle of the shadow
|  distance
:  <positive_integer>
:  5
:  Distance of the shadow

# EXAMPLES

To set the default values that are applied to all further items, put something
similar to this in the configuration file before all *--set* commands:

```
sketchybar --default \\
	icon.font="Hack Nerd Font:Bold:17.0"  \\
	icon.color=0xffffffff                 \\
	label.font="Hack Nerd Font:Bold:14.0" \\
	label.color=0xffffffff                \\
	padding_left=5                        \\
	padding_right=5                       \\
	label.padding_left=4                  \\
	label.padding_right=4                 \\
	icon.padding_left=4                   \\
	icon.padding_right=4
```

To add a simple clock item on the left part of the bar:

```
sketchybar --add item clock left \\
	--set clock update_freq=60 icon.drawing=off \\
		script='sketchybar --set $NAME label="$(date "+%a %m%d %H%M")"'
```

# SEE ALSO

*sketchybar*(1)++
*sketchybar*(5)++
*sketchybar-components*(5)++
*sketchybar-popup*(5)++
*sketchybar-events*(5)++
*sketchybar-query*(5)++
*sketchybar-animate*(5)++
*sketchybar-types*(5)++
*sketchybar-tips*(5)
