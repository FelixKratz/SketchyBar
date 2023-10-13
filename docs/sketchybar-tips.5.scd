sketchybar-tips(5)

# NAME

sketchybar-tips(5) - useful configuration tips and tricks 

# TIPS & TRICKS

## Batching of configuration commands

It is possible to batch commands together into a single call to sketchybar.
This can be helpful to keep the configuration file a bit cleaner and also to
reduce startup times. Consider these 5 individual configuration calls to
sketchybar:
```
sketchybar --bar position=top
sketchybar --bar margin=5
sketchybar --add item demo left
sketchybar --set demo label=Hello
sketchybar --subscribe demo system_woke
```

After each configuration command, the bar is redrawn (if needed). Thus it is
faster to append these calls into a single command as in
```
sketchybar --bar position=top \\
		margin=5 \\
	--add item demo left \\
	--set demo label=Hello \\
	--subscribe demo system_woke
```

The use of backslashes at the end of the first four lines is a posix-compliant
(includes bash, zsh) way in shell scripting to join lines together. It should
not be followed by a whitespace.

## Using arrays for cleaner configuration

Assume bash. Consider the bar configuration command
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

You can rewrite this as an array to get rid of the backslashes and pass the
contents of the array to the *--bar* command as in
```
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

## Debugging problems

If you are experiencing problems with the configuration process, it might be
helpful to work through the following steps:

. Start sketchybar directly from the commandline to see the verbose error or,
  warning messages.
. Make sure you have no trailing whitespaces after the newline escape character
  *\\*.
. Make sure your scripts are made executable (that is via chmod +x script.sh).
. Reduce the configuration to a minimal example and narrow down the problematic
  region.
. Try running erroneous scripts directly in the command line.
. Query sketchybar for relevant properties and use them to deduce the,
  problems root cause.
. Create and file an issue at https://github.com/FelixKratz/SketchyBar/issues.
  A second pair of eyes might now be the only thing that helps. 

## Color picker 

Sketchybar uses the argb hex color format, which means: *0xAARRGGBB* encodes a
color. If you are looking for colors, check out the color picker at
https://felixkratz.github.io/SketchyBar/config/bar

## Finding icons

The default font sketchybar uses is the *Hack Nerd Font*. This means all
*Nerdfont* icons can be used. Refer to the Nerdfont cheat-sheet at
https://www.nerdfonts.com/cheat-sheet to find new icons.

Additionally, it is possible to use other icons and glyphs from different fonts,
such as the *sf-symbols* (see https://developer.apple.com/sf-symbols) from Apple.
Those symbols can be installed via brew:
```
brew install --cask sf-symbols
```

After installing this package, an app called *SF Symbols* will be available
where you can find all the available icons. Once you find a fitting icon, right
click it, select _Copy Symbol_ and paste it in the relevant configuration file.

If you are looking for stylised app icons, you might want to checkout the
excellent community maintained app-icon-font for sketchybar (see
https://github.com/kvndrsslr/sketchybar-app-font).

## Multiple bars

It is possible to have multiple independent instances of SketchyBar running.
This is possible by changing the *argv[0]* of the sketchybar program. This is
very easy: do symlink the sketchybar binary with a different name (e.g.
*bottom_bar*) as in
```
ln -s $(which sketchybar) $(dirname $(which sketchybar))/bottom_bar
```

This symlink can now be used to spawn and target an additional bar. For this
bar, you do not call *sketchybar --bar color=0xffff0000* but rather
*bottom_bar --bar color=0xffff0000*. You also start it by running *bottom_bar*
in the command line.

The config path for this additional bar is in _$HOME/.config/bottom_bar/_. Of
course *bottom_bar* is only an example and can be freely replaced with any other
identifier.

You can bootstrap the additional bar by invoking it at the very bottom of your
main bar's sketchybarrc.

## Performance optimizations

Sketchybar can be configured to have a *very* small performance footprint. We
highlight some optimizations that can be used to reduce the footprint further:
- batch together configuration commands where possible
- set *updates=when_shown* for items that do not need to run their script if
  they are not rendered
- reduce the *update_freq* of scripts and aliases, and use event-driven
  scripting whenever possible
- do not add *aliases* to apps that are not always running; otherwise sketchybar
  searches for them continuously
- (Advanced, only >=v2.9.0) use compiled *mach_helper* programs that
  directly interface with sketchybar. An example for performance sensitive
  tasks can be found at https://github.com/FelixKratz/SketchyBarHelper

# SEE ALSO

*sketchybar*(1)++
*sketchybar*(5)++
*sketchybar-items*(5)++
*sketchybar-components*(5)++
*sketchybar-popup*(5)++
*sketchybar-events*(5)++
*sketchybar-query*(5)++
*sketchybar-animate*(5)++
*sketchybar-types*(5)
