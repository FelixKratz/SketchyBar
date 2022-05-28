---
id: animations
title: Animations
sidebar_position: 1
---
## Animating the bar (Experimental)
All transitions between `<argb_hex>`, `<integer>` and `<positive_integer>`
values can be animated, by prepending the animation command in front of any
regular `--set` or `--bar` command:

```bash
sketchybar --animate <curve> <duration> \
           --bar <property>=<value> ... <property>=<value> \
           --set <property>=<value> ... <property>=<value>
```
where the `<curve>` is any of the animation curves:
- `linear`, `tanh`, `sin`, `exp`, `bounce`, `overshoot`

and the `<duration>` is a positive integer quantifying the number of animation
steps.

The animation system *always* animates between all *current* values and the
values specified in a configuration command (i.e. `--bar` or `--set` commands).

## Examples

### Animate the startup
This example animates the bar appear smoothly from the top of the screen on
startup.

- In the initial bar configuration command in `sketchybarrc` set a bar
`y_offset=-32`, such that the bar is originally off screen:

```bash
sketchybar --bar y_offset=-32
```

- At the end of `sketchybarrc` (after `sketchybar --update`) add the
animation:

```bash
sketchybar --animate tanh 30 --bar y_offset=0
```

### Animate color change on space change
- Add a custom script to the space component and set the `color` and
`highlight_color`, e.g.:
```bash
sketchybar --set <space name> icon.highlight_color=0xff9dd274 icon.color=0xffffffff script=<path to space.sh>
```
- Create the custom space script `space.sh` and in it handle the space update
with an animation:
```bash
sketchybar --animate sin 50 --set $NAME icon.highlight=$SELECTED
```
