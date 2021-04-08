# Changelog

All notable changes to this project will be documented in this file.

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased](https://github.com/cmacrae/spacebar/compare/v1.2.1...HEAD)

**Changed**
- Fixed a bug introduced in Big Sur where buffer reads were incorrect
- Improved efficiency of bar's initialisation
- Fixed DoNotDisturb indicator on Big Sur

**Added**
- New `left|center|right` shell sections: display custom text based on shell pipelines
- Option to turn focused window title display on or off (thanks [@Norviah](https://github.com/Norviah)!)
- Option to turn the spaces indicator on or off
- Option to turn the clock on or off
- Option to turn the power indicator on or off
- Option to turn the DoNotDisturb indicator on or off
- Option to set the padding between the first/last item and the left/right edge of the display
- Default to "Solid" style of Font Awesome 5 Free icon font
- Option to display spaces for all displays, including: optional separator, secondary & tertiary space indicator colours in relation to display
- Option to specify only drawing one bar on the main display or multiple bars, one for each display (this aligns with yabai's `external_bar` option)
- Provide a [Nix Flake](https://nixos.wiki/wiki/Flakes)

## [1.2.1](https://github.com/cmacrae/spacebar/releases/tag/v1.2.1) - 2020-11-18

**Changed**
- Fixed a bug where querying for the value of `space_icon_strip` would set it to default (thanks [@jraregris](https://github.com/jraregris))

## [1.2.0](https://github.com/cmacrae/spacebar/releases/tag/v1.2.0) - 2020-11-18

**Added**
- Spacing configuration options

## [1.1.1](https://github.com/cmacrae/spacebar/releases/tag/v1.1.1) - 2020-07-21

**Changed**
- Fixed a bug where long window titles would draw over the right status area
- Padding between items in the status area based on current values

## [1.1.0](https://github.com/cmacrae/spacebar/releases/tag/v1.1.0) - 2020-07-17

**Added**
- Height configuration option

## [1.0.0](https://github.com/cmacrae/spacebar/releases/tag/v1.0.0) - 2020-07-16

**Added**
- Option to position at the top or bottom of the screen
- Individual colour settings for each icon in the right strip (`dnd`, `power`, `clock`)
- DoNotDisturb indicator

**Changed**
- Current space indicated by colouring the glyph
- Removal of underlines
- Fixed flicker bug when changing monitor focus (thanks [@tom-auger](https://github.com/tom-auger))

## Pre-1.0.0
This changelog was not kept up to date prior to `1.0.0`.  
See the commit log for more information.
