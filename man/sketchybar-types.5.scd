sketchybar-types(5)

# NAME

sketchybar-types - nomenclature of all value types

# DESCRIPTION

The nomenclature of all value types used in the other *sketchybar*(5) man pages
is described here.

# TYPE NOMENCLATURE

[[ *type*
:< *values*
|  <boolean>
:  on, off, yes, no, true, false, 1, 0, toggle
|  <argb_hex>
:  Color as an 8 digit hex with alpha, red, green and blue channels
|  <path>
:  An absolute file path
|  <string>
:  Any UTF-8 string or symbol
|  <float>
:  A floating point number
|  <integer>
:  An integer
|  <positive_integer>
:  A positive integer
|  <positive_integer list>
:  A comma separated list of positive integers

## Further <boolean> operations

All <boolean> properties can be negated with an exclamation mark, e.g. *!on*.
Note that you may have to quote the negated expression in certain shells.

## Further <argb_hex> operations

All colors (i.e. all fields where the value type is <argb_hex>) can additionally
be accessed to change specific channels like this:

[[ *color_property*
:[ *value*
:[ *default*
:< *description*
|  alpha
:  <float>
:  1.0
:  The alpha channel of the color (0 to 1)
|  red
:  <float>
:  1.0
:  The red channel of the color (0 to 1)
|  green
:  <float>
:  1.0
:  The green channel of the color (0 to 1)
|  blue
:  <float>
:  1.0
:  The blue channel of the color (0 to 1)

# EXAMPLES

If the alpha channel of the bar color is the only change needed, execute
	sketchybar --bar color.alpha=0.5

# SEE ALSO

*sketchybar*(1)++
*sketchybar*(5)++
*sketchybar-items*(5)++
*sketchybar-components*(5)++
*sketchybar-popup*(5)++
*sketchybar-events*(5)++
*sketchybar-query*(5)++
*sketchybar-animate*(5)++
*sketchybar-tips*(5)
