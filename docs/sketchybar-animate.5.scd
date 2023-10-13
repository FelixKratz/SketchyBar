sketchybar-animate(5)

# NAME

sketchybar-animate - animate the bar, items, etc.

# SYNOPSIS

*sketchybar --animate* _CURVE_ _DURATION_ *--bar* _SETTING_=_VALUE_ ...
_SETTING_=_VALUE_ ++
*sketchybar --animate* _CURVE_ _DURATION_ *--set* _PROPERTY_=_VALUE_ ...
_PROPERTY_=_VALUE_

# DESCRIPTION

All transitions between argb_hex, integer and positive_integer values can be
animated, by prepending the animation command in front of any regular *--set*
or *--bar* command.

A series of animations can be chained together as many as desired. The
animation function can also be changed in between. This is a nice way to
create custom animations with key-frames.

Other properties can also be made to wait for their animation until another
animation is finished. This is done by simply setting the property that
should wait to its current value in the first animation.

A superseding non-animated *--set* command targeting a currently animated
property will cancel the animation queue and immediately set the value.

A superseding animated *--set* command targeting a currently animated
property will cancel the animation queue and immediately begin with the new
animation beginning at the current state.

# OPTIONS

*--animate* _CURVE_ _DURATION_ *--bar* _SETTING_=_VALUE_ ... _SETTING_=_VALUE_ ++
*--animate* _CURVE_ _DURATION_ *--set* _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_

	Animate the bar or an item. _CURVE_ can be any of the following animation
	curves: *linear*, *quadratic*, *tanh*, *sin*, *exp*, *circ*. _DURATION_ is
	a positive integer quantifying the number of animation steps. The duration
	is the frame count on a 60 Hz display such that the temporal duration of the
	animation in seconds is given by _DURATION_ / 60.

The animation system *always* animates between all *current* values and the
values specified in a configuration command (i.e. `--bar` or `--set` commands).

# EXAMPLES

To chain two or more animations together, simply change the property
multiple times in a single call. Consider
```
sketchybar --animate sin 30 --bar y_offset=10 y_offset=0
```
This will animate the bar to the first offset and then to the second offset
afterwards.

# SEE ALSO

*sketchybar*(1)++
*sketchybar*(5)++
*sketchybar-items*(5)++
*sketchybar-components*(5)++
*sketchybar-popup*(5)++
*sketchybar-events*(5)++
*sketchybar-query*(5)++
*sketchybar-types*(5)++
*sketchybar-tips*(5)
