---
id: types
title: Type Nomenclature
sidebar_position: 1
---
## Type nomenclature

| `type`                    | `values`                                                         |
| -----                     | ---------                                                        |
| `<boolean>`               | `on`, `off`, `yes`, `no`, `true`, `false`, `1`, `0`, `toggle`    |
| `<argb_hex>`              | Color as an 8 digit hex with alpha, red, green and blue channels |
| `<path>`                  | An absolute file path                                            |
| `<string>`                | Any UTF-8 string or symbol                                       |
| `<float>`                 | A floating point number                                          |
| `<integer>`               | An integer                                                       |
| `<positive_integer>`      | A positive integer                                               |
| `<positive_integer list>` | A comma separated list of positive integers                      |

### Further `<boolean>` operations
All `<boolean>` properties can be negated with an exclamation mark, e.g. `!on`.

### Further `<argb_hex>` operations
All colors (i.e. all fields where the value type is `<argb_hex>`) can
additionally be accessed to change specific channels like this:

| <color_property\>  | <value\>     | default      | description                             |
| :-------:          | :------:     | :-------:    | -----------                             |
| `alpha`            | `<float>`    | `1.0`        | The alpha channel of the color (0 to 1) |
| `red`              | `<float>`    | `1.0`        | The red channel of the color (0 to 1)   |
| `green`            | `<float>`    | `1.0`        | The green channel of the color (0 to 1) |
| `blue`             | `<float>`    | `1.0`        | The blue channel of the color (0 to 1)  |

So for example, if I want to only change the alpha channel of the bars color I would use
```bash
sketchybar --bar color.alpha=0.5
```
