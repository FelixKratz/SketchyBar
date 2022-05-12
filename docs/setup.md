---
id: setup
title: Setup
sidebar_position: 1
---

## Installation
### Stable Version
```bash
brew tap FelixKratz/formulae
brew install sketchybar
```
Do not forget to copy the example configuration files to your home directory (the brew installation specific commands are listed in the caveats section after the brew install is finished).

Run the bar via
```bash
brew services start sketchybar
```

### Plugins and Fonts
When you use additional plugins, make sure that they are referenced in the rc with the correct path and that they are made executable via
```bash
chmod +x name/of/plugin.sh
```
The default plugin folder is located in `~/.config/sketchybar/plugins`.
All plugins must work with absolute paths because relative paths will not be resolved correctly.
Have a look at the [discussion](https://github.com/FelixKratz/SketchyBar/discussions/12) about plugins and share your own if you want to.
You should of course vet the code from all plugins before executing them to make sure they are not harming your computer.

If you have problems with missing fonts you might need to install the Hack Nerd Font:
```bash
brew tap homebrew/cask-fonts
brew install --cask font-hack-nerd-font
```

### Compile from source
It is easy to compile the project from source:

- Install xCode commandline tools:
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
possible to generate an arm64/x86 only binary via:

```bash
make arm64
```
or
```bash
make x86
```
Compiling on older macOS (pre Big Sur) versions should always be done via `make x86`.

## Uninstall
Uninstall via `brew`:
```bash
brew uninstall sketchybar
brew untap FelixKratz/formulae
```
