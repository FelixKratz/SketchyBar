---
id: animations
title: Animations
sidebar_position: 1
---
## Animating the bar (Experimental)
In the current stable release v2.6.0 animations are still experimental.
The current HEAD improves speed and other animation properties drastically.

All transitions between `<argb_hex>`, `<integer>` and `<positive_integer>`
values can be animated, by prepending the animation command in front of any
regular `--set` or `--bar` command:

```bash
sketchybar --animate <curve> <duration> \
           --bar <property>=<value> ... <property>=<value> \
           --set <name> <property>=<value> ... <property>=<value>
```
where the `<curve>` is any of the animation curves:
- `linear`, `tanh`, `sin`, `exp`, `bounce`, `overshoot`

and the `<duration>` is a positive integer quantifying the number of animation
steps.

The animation system *always* animates between all *current* values and the
values specified in a configuration command (i.e. `--bar` or `--set` commands).

### Perform multiple animations chained together (Only on HEAD)
If you want to chain two or more animations together, you can do so by simply
changing the property multiple times, e.g.:
```bash
sketchybar --animate sin 30 --bar y_offset=10 y_offset=0
```
will animate the bar to the first offset and after that to the second offset.
You can chain together as main animations as you like and you can change the
animation function in between. This is a nice way to create custom animations
with key-frames. You can also make other properties wait with their animation
till another animation is finished, by simply setting the property that should
wait to its current value in the first animation.
