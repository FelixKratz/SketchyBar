sketchybar(5)

# NAME

sketchybar - configuration of the bar

# SYNOPSIS

*sketchybar --bar* _SETTING_=_VALUE_ ... _SETTING_=_VALUE_

# SETTINGS

The possible settings to configure the bar include

[[ *setting*
:[ *value*
:[ *default*
:< *description*
|  color
:  <argb_hex>
:  0x44000000
:  Color of the bar
|  border_color
:  <argb_hex>
:  0xffff0000
:  Color of the bar's border
|  position
:  top, bottom
:  top
:  Position of the bar on the screen
|  height
:  <integer>
:  25
:  Height of the bar
|  margin
:  <integer>
:  0
:  Margin around the bar
|  y_offset
:  <integer>
:  0
:  Vertical offset of the bar from its default position
|  corner_radius
:  <positive_integer>
:  0
:  Corner radius of the bar
|  border_width
:  <positive_integer>
:  0
:  Border width of the bar's border
|  blur_radius
:  <positive_integer>
:  0
:  Blur radius applied to the background of the bar
|  padding_left
:  <positive_integer>
:  0
:  Padding between the left bar border and the leftmost item
|  padding_right
:  <positive_integer>
:  0
:  Padding between the right bar border and the rightmost item
|  notch_width
:  <positive_integer>
:  200
:  The width of the notch to be accounted for on the internal display
|  notch_offset
:  <positive_integer>
:  0
:  Additional y_offset exclusively applied to notched screens
|  display
:  main, all, <positive_integer list>
:  all
:  Display to show the bar on
|  hidden
:  <boolean>, current
:  off
:  If all / the current bar is hidden
|  topmost
:  <boolean>, window
:  off
:  If the bar should be drawn on top of everything, or on top of all windows
|  sticky
:  <boolean>
:  off
:  Makes the bar sticky (only use with disabled space change animations)
|  font_smoothing
:  <boolean>
:  off
:  If fonts should be smoothened
|  shadow
:  <boolean>
:  off
:  If the bar should draw a shadow


For details about *sticky*, see
https://github.com/FelixKratz/SketchyBar/issues/220

# EXAMPLES

```
sketchybar --bar \\
	height=32        \\
	blur_radius=30   \\
	position=top     \\
	sticky=off       \\
	padding_left=10  \\
	padding_right=10 \\
	color=0x15ffffff
```

# SEE ALSO

*sketchybar*(1)++
*sketchybar-items*(5)++
*sketchybar-components*(5)++
*sketchybar-popup*(5)++
*sketchybar-events*(5)++
*sketchybar-query*(5)++
*sketchybar-animate*(5)++
*sketchybar-types*(5)++
*sketchybar-tips*(5)
