---
id: tricks
title: Tips & Tricks
sidebar_position: 1
---
import SketchExample from "../../src/pages/picker.js"

## Batching of configuration commands
It is possible to batch commands together into a single call to sketchybar, this can be helpful to
keep the configuration file a bit cleaner and also to reduce startup times.
Assume 5 individual configuration calls to sketchybar:
```bash
sketchybar --bar position=top
sketchybar --bar margin=5
sketchybar --add item demo left
sketchybar --set demo label=Hello
sketchybar --subscribe demo system_woke
```
after each configuration command the bar is redrawn (if needed), thus it is more perfomant to append these calls into a single command like so:
```bash
sketchybar --bar position=top           \
                 margin=5               \
           --add item demo left         \
           --set demo label=Hello       \
           --subscribe demo system_woke
```
The backslash at the end of the first 4 lines is the default bash way to join lines together and should not be followed by a whitespace.  

## Color Picker
SketchyBar uses the argb hex color format, which means: `0xAARRGGBB` encodes a
color.

<SketchExample />

## Finding Icons
The default font sketchybar uses is the *Hack Nerd Font* which means all *Nerdfont* icons can be used.
Refer to the *Nerdfont* [cheat-sheet](https://www.nerdfonts.com/cheat-sheet) to find new icons.

Additionally, it is possible to use other icons and glyphs from different fonts, such as the [sf-symbols](https://developer.apple.com/sf-symbols/)
from apple. Those symbols can be installed via brew:
```bash
brew install --cask sf-symbols
```
After installing this package, an app called `SF Symbols` will be available where you can find all the available icons.
Once you find a fitting icon, right click it, select *Copy Symbol* and paste it in the relevant configuration file.
My [personal setup](https://github.com/FelixKratz/dotfiles) uses *SF Symbols* as well.

## Performance optimizations
*SketchyBar* can be configured to have a *very* small performance footprint. In the following I will highlight some optimizations that can be used to reduce the footprint further. 

* Batch together configuration commands where ever possible.
* Set items to be *lazy*, e.g. I have an alias component in my bar that updates every *2* seconds, thus I set all *non-reactive* items to *lazy=on*, 
and only the ones that should react to change instantaneously to *lazy=off*.
* Set *updates=when_shown* for items that do not need to run their script if they are not rendered.
* Reduce the *update_freq* of *scripts* and *aliases* and use event-driven scripting when ever possible.
* Do not add *aliases* to apps that are not always running, otherwise sketchybar searches for them continously.

