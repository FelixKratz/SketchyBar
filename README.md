<!-- Please be careful editing the below HTML, as GitHub is quite finicky with anything that looks like an HTML tag in GitHub Flavored Markdown. -->
<p align="center">
  <b>Status Bar for the Mac.</b>
</p>
<p align="center">
  <a href="https://travis-ci.org/somdoron/spacebar">
    <img src="https://travis-ci.org/somdoron/spacebar.svg?branch=master" alt="CI Status Badge">
  </a>
  <a href="https://github.com/somdoron/spacebar/blob/master/LICENSE.txt">
    <img src="https://img.shields.io/github/license/somdoron/spacebar.svg?color=green" alt="License Badge">
  </a>
  <a href="https://github.com/spacebar/blob/blob/master/CHANGELOG.md">
    <img src="https://img.shields.io/badge/view-changelog-green.svg" alt="Changelog Badge">
  </a>
  <a href="https://github.com/somdoron/spacebar/releases">
    <img src="https://img.shields.io/github/commits-since/somdoron/spacebar/latest.svg?color=green" alt="Version Badge">
  </a>
</p>

## About

spacebar is a status bar for [&nearr;&nbsp;yabai][gh-yabai] tiling window management.

## Installation

A codesigned binary release of spacebar can be installed using Homebrew from the tap somdoron/formulae. 
```
brew install somdoron/formulae/spacebar
```

Open System Preferences.app and navigate to Security & Privacy, then Privacy, then Accessibility. Click the lock icon at the bottom and enter your password to allow changes to the list. Starting with `brew services start spacebar` will prompt the user to allow spacebar accessibility permissions. Check the box next to spacebar to allow accessibility permissions.

To run space, simply start it:
```
brew services start spacebar
```

## Configuration

The per-user spacebar configuration file must be executable; it is just a shell script that's ran before spacebar launches. It must be placed at one of the following places (in order):

* $XDG_CONFIG_HOME/spacebar/spacebarrc
* $HOME/.config/spacebar/spacebarrc
* $HOME/.spacebarrc

Create empty configuration file and make it executable:
```
touch ~/.config/spacebar/spacebarrc
chmod +x ~/.config/spacebar/spacebarrc
```

All of the configuration options can be changed at runtime as well.

Example configuration:
```
spacebar -m config status_bar_text_font         "Helvetica Neue:Bold:12.0"
spacebar -m config status_bar_icon_font         "Font Awesome 5:Regular:12.0"
spacebar -m config status_bar_background_color  0xff202020
spacebar -m config status_bar_foreground_color  0xffa8a8a8
spacebar -m config status_bar_space_icon_strip  I II III IV V VI VII VIII IX X
spacebar -m config status_bar_power_icon_strip   
spacebar -m config status_bar_space_icon        
spacebar -m config status_bar_clock_icon        
```

- Sample configuration files can be found in the [&nearr;&nbsp;examples][spacebar-examples] directory. Refer to the [&nearr;&nbsp;documentation][spacebar-docs].

## Integration with yabai

Add the following to your yabai configuration, so yabai won't draw over the status bar.

```
yabai -m config top_padding 26
```

## Debug output and error reporting

In the case that something is not working as you're expecting, please make sure to take a look in the output and error log. To enable debug output make sure that your configuration file contains `spacebar -m config debug_output on` or that spacebar is launched with the `--verbose` flag. If you are running through brew services the log files can be found in the following directory:

```
# directory containing log files (HOMEBREW_PREFIX defaults to /usr/local unless you manually specified otherwise)
HOMEBREW_PREFIX/var/log/spacebar/

# view the last lines of the error log 
tail -f /usr/local/var/log/spacebar/spacebar.err.log

# view the last lines of the debug log
tail -f /usr/local/var/log/spacebar/spacebar.out.log
```

## Upgrade

```
brew services stop spacebar
brew upgrade spacebar
brew services start spacebar
```

## Requirements and Caveats

Please read the below requirements carefully.
Make sure you fulfil all of them before filing an issue.

|Requirement|Note|
|-:|:-|
|Operating&nbsp;System|macOS Catalina 10.15.0+ is supported.|
|Accessibility&nbsp;API|spacebar must be given permission to utilize the Accessibility API and will request access upon launch. The application must be restarted after access has been granted.|

Please also take note of the following caveats.

|Caveat|Note|
|-:|:-|
|Code&nbsp;Signing|When building from source (or installing from HEAD), it is recommended to codesign the binary so it retains its accessibility and automation privileges when updated or rebuilt.|
|Mission&nbsp;Control|In the Mission Control preferences pane in System Preferences, the setting "Automatically rearrange Spaces based on most recent use" should be disabled.|

## License and Attribution

spacebar is licensed under the [&nearr;&nbsp;MIT&nbsp;License][spacebar-license], a short and simple permissive license with conditions only requiring preservation of copyright and license notices.
Licensed works, modifications, and larger works may be distributed under different terms and without source code.

Thanks to [@koekeishiya][gh-koekeishiya] for creating yabai.

## Disclaimer

Use at your own discretion.
I take no responsibility if anything should happen to your machine while trying to install, test or otherwise use this software in any form.

<!-- Project internal links -->
[spacebar-license]: LICENSE.txt
[spacebar-examples]: https://github.com/somdoron/spacebar/tree/master/examples
[spacebar-docs]: https://github.com/somdoron/spacebar/blob/master/doc/spacebar.asciidoc

<!-- Links to other GitHub projects/users -->
[gh-koekeishiya]: https://github.com/koekeishiya
[gh-yabai]: https://github.com/koekeishiya/yabai
