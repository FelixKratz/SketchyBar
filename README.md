<p align="center">
  <img src="images/Sbar.svg" />
</p>

<p align="center">
<a href="https://opensource.org/licenses/"><img src="https://img.shields.io/badge/License-GPL%20v3-blue.svg"></a>
<a href="https://github.com/FelixKratz/SketchyBar/releases"><img src="https://img.shields.io/github/v/release/FelixKratz/SketchyBar.svg?style=flat&color=orange" /></a>
<a href="https://github.com/FelixKratz/SketchyBar/releases"><img src="https://img.shields.io/github/commits-since/FelixKratz/SketchyBar/latest.svg?color=orange"></a>
<a href="https://en.wikipedia.org/wiki/Free_and_open-source_software"><img src="https://img.shields.io/badge/FOSS-100%25-green.svg?style=flat"></a>
</p>

<p align="center">
This bar project aims to create a highly flexible, customizable, fast and powerful status bar replacement for users that like playing around with
shell scripts.
</p>

![](images/example.png)
<p align="center">
More example setups <a href="https://github.com/FelixKratz/SketchyBar/discussions/47">here</a>. Full documentation <a href="https://felixkratz.github.io/SketchyBar/config/bar">here</a>.
</p>

## Features

* Performance friendly
* No accessibility permissions needed
* Fully scriptable
* Fully configurable
* Supports drawing native macOS menu bar applications
* Powerful event system
* Popup Menus
* Mouse Support
* Support for graphs
* Per display and per space individualization

## Documentation
For the full documentation of all commands and properties please refer to the [website](https://felixkratz.github.io/SketchyBar/config/bar)
or if you prefer a single document see the markdown [docs](https://github.com/FelixKratz/SketchyBar/blob/master/DOCS.md).

## Installation
### Brew Install
```bash
brew tap FelixKratz/formulae
brew install sketchybar
```
Do not forget to copy the example configuration files to your home directory
(the brew installation specific commands are listed in the caveats section after the brew install is finished).
The default configuration is intentionally sparse, so if you are looking for something more sophisticated as a starting point, you might want to look at
[this discussion](https://github.com/FelixKratz/SketchyBar/discussions/47) or my personal [dotfiles](https://github.com/FelixKratz/dotfiles).

Run the bar automatically at startup:
```bash
brew services start sketchybar
```
or in the command line with verbose output:
```bash
sketchybar
```

### Fonts
The default sketchybar font is the Hack Nerd Font:
```bash
brew tap homebrew/cask-fonts
brew install --cask font-hack-nerd-font
```
if you experience missing icons you might need to install it. Any font
of your liking can be used in sketchybar.

### Plugins
When you use/create additional plugins, make sure that they are made executable via
```bash
chmod +x name/of/plugin.sh
```
If you run sketchybar from the command line directly with the command `sketchybar` you will see
all outputs and error messages from your scripts.

The default plugin folder is located in `~/.config/sketchybar/plugins`.
Plugins need to be referenced with absolute paths because relative paths will not be resolved correctly.
Have a look at the [discussion](https://github.com/FelixKratz/SketchyBar/discussions/12) for plugins and share your own if you want to.
You should of course vet the code from all plugins before executing them to make sure they are not harming your computer.

## Credits
This project was forked from *[spacebar](https://github.com/cmacrae/spacebar)* and completely reimagined and rewritten. <br>
The original idea is based on the status bar that was included in *[yabai](https://github.com/koekeishiya/yabai)* before getting removed.
