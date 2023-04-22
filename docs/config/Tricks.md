---
id: tricks
title: Tips & Tricks
sidebar_position: 1
---
import SketchExample from "../../src/pages/picker.js"

## Batching of configuration commands
It is possible to batch commands together into a single call to *SketchyBar*, this can be helpful to
keep the configuration file a bit cleaner and also to reduce startup times.
Assume 5 individual configuration calls to *SketchyBar*:
```bash
sketchybar --bar position=top
sketchybar --bar margin=5
sketchybar --add item demo left
sketchybar --set demo label=Hello
sketchybar --subscribe demo system_woke
```
after each configuration command the bar is redrawn (if needed), thus it is
faster to append these calls into a single command like so:
```bash
sketchybar --bar position=top           \
                 margin=5               \
           --add item demo left         \
           --set demo label=Hello       \
           --subscribe demo system_woke
```
The backslash at the end of the first 4 lines is the default bash way to join lines together and should not be followed by a whitespace.  

### Using bash arrays for cleaner configuration
Lets assume this bar configuration command (from the default config):
```bash
sketchybar --bar height=32        \
                 blur_radius=30   \
                 position=top     \
                 sticky=off       \
                 padding_left=10  \
                 padding_right=10 \
                 color=0x15ffffff
```
We can rewrite this as a bash array to get rid of the backslashes and pass the
contents of the array to the `--bar` command:
```bash
bar=(
  height=32
  blur_radius=30
  position=top
  sticky=off
  padding_left=10
  padding_right=10
  color=0x15ffffff
)

sketchybar --bar "${bar[@]}"
```

## Debugging Problems
If you are experiencing problems with the configuration of *SketchyBar* it might be helpful to work through the following steps:
* 1.) Start `sketchybar` directly from the commandline to see the verbose error/warning messages
* 2.) Make sure you have no trailing whitespaces after the bash newline escape char `\`
* 3.) Make sure your scripts are made executable via: `chmod +x script.sh`
* 4.) Reduce the configuration to a minimal example and narrow down the problematic region
* 5.) Try running erroneous scripts directly in the commandline
* 6.) Query *SketchyBar* for relevant properties and use them to deduce the problems root cause
* 7.) Create an [Issue](https://github.com/FelixKratz/SketchyBar/issues) on GitHub, a second pair of eyes might now be the only thing that helps

## Color Picker
SketchyBar uses the argb hex color format, which means: `0xAARRGGBB` encodes a
color.

<SketchExample />

## Finding Icons
The default font *SketchyBar* uses is the *Hack Nerd Font* which means all *Nerdfont* icons can be used.
Refer to the *Nerdfont* [cheat-sheet](https://www.nerdfonts.com/cheat-sheet) to find new icons.

Additionally, it is possible to use other icons and glyphs from different fonts,
such as the [sf-symbols](https://developer.apple.com/sf-symbols/) from apple.
Those symbols can be installed via brew:
```bash
brew install --cask sf-symbols
```
After installing this package, an app called `SF Symbols` will be available where you can find all the available icons.
Once you find a fitting icon, right click it, select *Copy Symbol* and paste it in the relevant configuration file.

If you are looking for stylised app icons you might want to checkout the excellent community maintained
[app-icon-font](https://github.com/kvndrsslr/sketchybar-app-font) for SketchyBar.

## Multiple Bars
It is possible to have multiple independent instances of SketchyBar running.
This is possible by changing the `argv[0]` of the sketchybar program. This is
very easy, e.g. by symlinking the sketchybar binary with a different name, e.g.
`bottom_bar`:
```bash
ln -s $(which sketchybar) $(dirname $(which sketchybar))/bottom_bar
```
This symlink can now be used to spawn and target an additional bar, i.e. for
this bar we do not call `sketchybar --bar color=0xffff0000`, but rather
`bottom_bar --bar color=0xffff0000` and start it by running `bottom_bar` in the
commandline.

The config path for this additional bar is in `$HOME/.config/bottom_bar/`.
Of course `bottom_bar` is only an example and can be freely replaced with any
other identifier.

## Performance optimizations
*SketchyBar* can be configured to have a *very* small performance footprint. In the following I will highlight some optimizations that can be used to reduce the footprint further. 

* Batch together configuration commands where ever possible.
* Set *updates=when_shown* for items that do not need to run their script if they are not rendered.
* Reduce the *update_freq* of *scripts* and *aliases* and use event-driven scripting when ever possible.
* Do not add *aliases* to apps that are not always running, otherwise *SketchyBar* searches for them continuously.
* (Advanced; Only >=v2.9.0) Use compiled `mach_helper` programs that directly interface with *SketchyBar* [example](https://github.com/FelixKratz/SketchyBarHelper) for performance sensitive tasks

