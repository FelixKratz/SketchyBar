---
id: setup
title: Setup
sidebar_position: 1
---

## Installation
### Prerequisite
SketchyBar will only work properly with the system setting "Displays have
separate Spaces" turned on (the default setting). The option is located
in *System Settings* -> *Desktop & Dock* -> *Displays have separate Spaces*.

### Brew Install
```bash
brew tap FelixKratz/formulae
brew install sketchybar
```
Copy the example configuration and make it executable:
```bash
mkdir -p ~/.config/sketchybar/plugins
cp $(brew --prefix)/share/sketchybar/examples/sketchybarrc ~/.config/sketchybar/sketchybarrc
cp -r $(brew --prefix)/share/sketchybar/examples/plugins/ ~/.config/sketchybar/plugins/
```
The default configuration is intentionally sparse, so if you are looking for something more sophisticated as a starting point, you might want to look at
[this discussion](https://github.com/FelixKratz/SketchyBar/discussions/47).

Run the bar automatically at startup:
```bash
brew services start sketchybar
```
or in the command line with verbose output:
```bash
sketchybar
```

It is possible to run sketchybar with a custom config file path (i.e. something
else than `$HOME/.config/sketchybar/sketchybarrc`) via:
```bash
sketchybar --config <path>
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

### Hiding the original macOS bar
- (Pre macOS Ventura) Hide the default macOS menu bar in *System Preferences* -> *Dock & Menu Bar*:
![hide_macOS_bar](/img/hide_menubar.png)
- (macOS Ventura) Hide the default macOS menu bar in *System Settings* -> *Desktop & Dock* -> *Automatically hide and show the menu bar* -> *Always*:
![hide_macOS_bar_ventura](/img/hide_macos_bar_ventura.png)
- (macOS Sonoma) Hide the default macOS menu bar in *System Settings* -> *Control Center* -> *Automatically hide and show the menu bar* -> *Always*:
![hide_macOS_bar_sonoma](/img/hide_macos_bar_sonoma.png)

### Compile from source
It is easy to compile the project from source:

- Install Xcode commandline tools:
```bash
xcode-select --install
```

- Clone the repository:
```bash
git clone https://github.com/FelixKratz/SketchyBar.git
```

- In the sketchybar folder run:
```bash
make
```

This will generate a universal binary with arm64 and x86 instructions. It is
possible to generate an arm64/x86 only binary via `make arm64` or `make x86`.
Compiling on older macOS (pre Big Sur) versions should always be done via `make x86`.

## Uninstall
Uninstall via `brew`:
```bash
brew uninstall sketchybar
brew untap FelixKratz/formulae
```
