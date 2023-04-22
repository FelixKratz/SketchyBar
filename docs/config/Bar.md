---
id: bar
title: Bar Properties
sidebar_position: 1
---
## Configuration of the bar
For an example configuration see the supplied default *sketchybarrc*.
The configuration file resides in `~/.config/sketchybar/sketchybarrc` and is a
regular script that gets executed when *SketchyBar* launches, everything
persistent should be set up in this script.

It is possible to play with properties in the commandline and change
them on the fly while the bar is running, once you find a fitting
value you can include it in the `sketchybarrc` file, such that the configuration
is restored on restart. When configuring *SketchyBar* it can be helpful to stop
the brew service and run `sketchybar` from the commandline directly to see all
relevant error messages and warnings directly.

The global bar properties can be configured by invoking:
```bash
sketchybar --bar <setting>=<value> ... <setting>=<value>
```

where possible settings are:

| <setting\>       | <value\>               | default      | description                                                 |
| :-------:        | :------:               | :-------:    | -----------                                                 |
| `color`          | `<argb_hex>`           | `0x44000000` | Color of the bar                                            |
| `border_color`   | `<argb_hex>`           | `0xffff0000` | Color of the bars border                                    |
| `position`       | `top`, `bottom`        | `top`        | Position of the bar on the screen                           |
| `height`         | `<integer>`            | `25`         | Height of the bar                                           |
| `margin`         | `<integer>`            | `0`          | Margin around the bar                                       |
| `y_offset`       | `<integer>`            | `0`          | Vertical offset of the bar from its default position        |
| `corner_radius`  | `<positive_integer>`   | `0`          | Corner radius of the bar                                    |
| `border_width`   | `<positive_integer>`   | `0`          | Border width of the bars border                             |
| `blur_radius`    | `<positive_integer>`   | `0`          | Blur radius applied to the background of the bar            |
| `padding_left`   | `<positive_integer>`   | `0`          | Padding between the left bar border and the leftmost item   |
| `padding_right`  | `<positive_integer>`   | `0`          | Padding between the right bar border and the rightmost item |
| `notch_width`    | `<positive_integer>`   | `200`        | The width of the notch to be accounted for on the internal display |
| `notch_offset`   | `<positive_integer>`   | `0`          | Additional `y_offset` exclusively applied to notched screens |
| `display`        | `main`, `all`, `<positive_integer list>`          | `all`        | Display to show the bar on                                  |
| `hidden`         | `<boolean>`, `current` | `off`        | If all / the current bar is hidden                          |
| `topmost`        | `<boolean>`            | `off`        | If the bar should be drawn on top of `everything`           |
| `sticky`         | `<boolean>`            | `off`        | Makes the bar sticky (only use with disabled space change animations [#220](https://github.com/FelixKratz/SketchyBar/issues/220)) |
| `font_smoothing` | `<boolean>`            | `off`        | If fonts should be smoothened                               |
| `shadow`         | `<boolean>`            | `off`        | If the bar should draw a shadow                             |

You can find the nomenclature for all the types [here](https://felixkratz.github.io/SketchyBar/config/types).
If you are looking for colors, check out the [color picker](https://felixkratz.github.io/SketchyBar/config/tricks#color-picker).
