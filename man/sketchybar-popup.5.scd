sketchybar-popup(5)

# NAME

sketchybar-popup - configuration of popup menus

# SYNOPSIS

*sketchybar --set* _NAME_ *popup*._PROPERTY_=_VALUE_

# DESCRIPTION

Popup menus are a powerful way to make further items accessible in a small popup
window below any bar item. Every item and component can have a popup menu.

# OPTIONS

*--set* _NAME_ *popup*._PROPERTY_=_VALUE_
	Change the popup properties of an item or a component. Inherits the usual
	*--set* command usage. A list of properties available can be found in
	PROPERTIES. See below.

	Items can be added to a popup menu by setting the position of those items to
	*popup*._NAME_ where _NAME_ is the name of the item containing the popup.

# PROPERTIES

[[ *popup_property*
:[ *value*
:[ *default*
:< *description*
|  drawing
:  <boolean>
:  off
:  If the popup should be rendered
|  horizontal
:  <boolean>
:  off
:  If the popup should render horizontally
|  topmost
:  <boolean>
:  on
:  If the popup should always be on top of all other windows
|  height
:  <positive_integer>
:  bar height
:  The vertical spacing between items in a popup
|  blur_radius
:  <positive_integer>
:  0
:  The blur applied to the popup background
|  y_offset
:  <integer>
:  0
:  Vertical offset applied to the popup
|  align
:  left, right, center
:  left
:  Alignment of the popup with its parent item in the bar
|  background.<background_property>
:  -
:  -
:  Popups have a background and support all properties


# EXAMPLES

To get a simple popup menu that allows us to either go to System Settings or
Activity Monitor, or lock the screen:

```
sketchybar --bar height=32 \\
	--add item apple.logo left \\
	--set apple.logo icon="#Popup#" \\
		click_script='sketchybar --set $NAME popup.drawing=toggle' \\
	--add item apple.preferences popup.apple.logo \\
	--set apple.preferences label="Settings" \\
		click_script="open -a 'System Settings';
			sketchybar --set apple.logo popup.drawing=off" \\
	--add item apple.activity popup.apple.logo \\
	--set apple.activity label="Activity" \\
		click_script="open -a 'Activity Monitor';
			sketchybar --set apple.logo popup.drawing=off" \\
	--add item apple.lock popup.apple.logo \\
	--set apple.lock label="Lock Screen" \\
		click_script="pmset displaysleepnow; 
			sketchybar --set apple.logo popup.drawing=off"
```

More examples be found at 
https://github.com/FelixKratz/SketchyBar/discussions/12?sort=new#discussioncomment-1843975

# SEE ALSO

*sketchybar*(1)++
*sketchybar*(5)++
*sketchybar-items*(5)++
*sketchybar-components*(5)++
*sketchybar-events*(5)++
*sketchybar-query*(5)++
*sketchybar-animate*(5)++
*sketchybar-types*(5)++
*sketchybar-tips*(5)
