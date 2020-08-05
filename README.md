<!-- Please be careful editing the below HTML, as GitHub is quite finicky with anything that looks like an HTML tag in GitHub Flavored Markdown. -->
<p align="center">
  <b>A minimal status bar for macOS</b>
</p>
<p align="center">
  <a href="https://travis-ci.org/cmacrae/spacebar">
    <img src="https://img.shields.io/travis/cmacrae/spacebar/master.svg?colorB=b6eb7a&colorC=e84a5f" alt="CI Status Badge">
  </a>
  <a href="https://github.com/cmacrae/spacebar/blob/master/LICENSE.txt">
    <img src="https://img.shields.io/github/license/cmacrae/spacebar.svg?color=a6dcef" alt="License Badge">
  </a>
  <a href="https://github.com/cmacrae/spacebar/blob/master/CHANGELOG.md">
    <img src="https://img.shields.io/badge/view-changelog-726a95.svg" alt="Changelog Badge">
  </a>
</p>
<p align="center">
  <a href="https://builtwithnix.org">
    <img src="https://img.shields.io/badge/Built_With-Nix-5277C3.svg?logo=nixos&labelColor=73C3D5" alt="Nix Badge">
  </a>
  <a href="https://github.com/cmacrae/spacebar/projects/1">
    <img src="https://img.shields.io/badge/Project-tasks-7fdbda.svg?logo=trello" alt="GitHub Project Badge">
  </a>
  <a href="https://github.com/cmacrae/spacebar/compare/v1.1.1...HEAD">
    <img src="https://img.shields.io/github/commits-since/cmacrae/spacebar/latest.svg?color=ea907a" alt="Version Badge">
  </a>
</p>

## About
spacebar is a minimal status bar for macOS. Ideal for use with tiling window managers like [yabai](https://github.com/koekeishiya/yabai).

<p align="center">
  <img src="https://i.imgur.com/SFe0ifD.png" alt="spacebar demo">
</p>

## Installation
A package and service to install and manage spacebar is provided in two flavours: [Homebrew](https://brew.sh) & [Nix](https://nixos.org).

### Homebrew
spacebar can be installed using Homebrew from the `cmacrae/formulae` tap
```
brew install cmacrae/formulae/spacebar
brew services start spacebar
```

### Nix
A package is generally available to Nix users on macOS who follow the `nixpkgs-unstable` channel.  
spacebar can be configured and managed in a declarative manner using the `services.spacebar` module in [nix-darwin](https://github.com/LnL7/nix-darwin)

### Accessibility Permissions
spacebar makes use of the macOS Accessibility APIs -  after starting spacebar, you should be prompted to grant access.  
Open System Preferences.app and navigate to Security & Privacy, then Privacy, then Accessibility. Click the lock icon at the bottom and enter your password to allow changes to the list. Check the box next to spacebar to allow accessibility permissions.

## Configuration
spacebar is configured by setting `config` properties via its messaging socket. Not only does this mean you can try out config changes live, it also means spacebar's configuration file is simply a shell script - usually just a sequence of `spacebar -m config <option> <value>` statements.  

spacebar's configuration file must executable and is looked for in the following locations (in this order) by default:
* `$XDG_CONFIG_HOME/spacebar/spacebarrc`
* `$HOME/.config/spacebar/spacebarrc`
* `$HOME/.spacebarrc`

### Getting started
To get started, create an empty configuration file and make it executable:
```
mkdir -p ~/.config/spacebar
touch ~/.config/spacebar/spacebarrc
chmod +x ~/.config/spacebar/spacebarrc
```

Here's a configuration taken from [the examples directory](examples):
```
#!/usr/bin/env sh

spacebar -m config position           top
spacebar -m config height             26
spacebar -m config spacing_left       25
spacebar -m config spacing_right      15
spacebar -m config text_font          "Helvetica Neue:Bold:12.0"
spacebar -m config icon_font          "Font Awesome 5 Free:Regular:12.0"
spacebar -m config background_color   0xff202020
spacebar -m config foreground_color   0xffa8a8a8
spacebar -m config space_icon_color   0xff458588
spacebar -m config power_icon_color   0xffcd950c
spacebar -m config battery_icon_color 0xffd75f5f
spacebar -m config dnd_icon_color     0xffa8a8a8
spacebar -m config clock_icon_color   0xffa8a8a8
spacebar -m config space_icon_strip   I II III IV V VI VII VIII IX X
spacebar -m config power_icon_strip    
spacebar -m config space_icon         
spacebar -m config clock_icon         
spacebar -m config dnd_icon           
spacebar -m config clock_format       "%d/%m/%y %R"

echo "spacebar configuration loaded.."
```
_Note: Ensure fonts are installed to use glyphs_

For further configuration documentation, please see [`man spacebar`](doc/spacebar.asciidoc)

### Declarative configuration with Nix
If you're using the `services.spacebar` module from [nix-darwin](https://github.com/LnL7/nix-darwin), you can configure spacebar like so:
```nix
{
  services.spacebar.enable = true;
  services.spacebar.package = pkgs.spacebar;
  services.spacebar.config = {
    position           = "top";
    height             = 26;
    spacing_left       = 25;
    spacing_right      = 15;
    text_font          = ''"Helvetica Neue:Bold:12.0"'';
    icon_font          = ''"Font Awesome 5 Free:Regular:12.0"'';
    background_color   = "0xff202020";
    foreground_color   = "0xffa8a8a8";
    space_icon_color   = "0xff458588";
    power_icon_color   = "0xffcd950c";
    battery_icon_color = "0xffd75f5f";
    dnd_icon_color     = "0xffa8a8a8";
    clock_icon_color   = "0xffa8a8a8";
    space_icon_strip   = "I II III IV V VI VII VIII IX X";
    power_icon_strip   = " ";
    space_icon         = "";
    clock_icon         = "";
    dnd_icon           = "";
    clock_format       = ''"%d/%m/%y %R"'';
  };
}
```

## Integration with yabai
yabai provides the `external_bar` config option. This can be used so yabai plays nice with spacebar.  
Take a look at this excerpt from the yabai man page
>       external_bar [<main|all|off>:<top_padding>:<bottom_padding>]
>           Specify top and bottom padding for a potential custom bar that you may be running.
>           main: Apply the given padding only to spaces located on the main display.
>           all:  Apply the given padding to all spaces regardless of their display.
>           off:  Do not apply any special padding.


So, if you like having spacebar at the bottom, you'd use `yabai -m config external_bar all:0:26`  

You can also use the command `spacebar -m config height` with no argument to get the current height, which you could then use in conjunction with `external_bar`:
```
SPACEBAR_HEIGHT=$(spacebar -m config height)
yabai -m config external_bar all:0:$SPACEBAR_HEIGHT
```

## Debug output and error reporting
In the case that something isn't working as you're expecting, please make sure to take a look in the output and error log. To enable debug output make sure that your configuration file contains `spacebar -m config debug_output on` or that spacebar is launched with the `--verbose` flag.

### Homebrew
If you're using the Homebrew service, the log files can be found in the following directory:
```
# directory containing log files (HOMEBREW_PREFIX defaults to /usr/local unless you manually specified otherwise)
$HOMEBREW_PREFIX/var/log/spacebar/

# view the last lines of the error log 
tail -f /usr/local/var/log/spacebar/spacebar.err.log

# view the last lines of the debug log
tail -f /usr/local/var/log/spacebar/spacebar.out.log
```

### Nix
If you're using the Nix service, you can set up debugging like so:
```nix
{
  services.spacebar.config.debug_output = "on";
  launchd.user.agents.spacebar.serviceConfig.StandardErrorPath = "/tmp/spacebar.err.log";
  launchd.user.agents.spacebar.serviceConfig.StandardOutPath = "/tmp/spacebar.out.log";
}
```

## Upgrading
To upgrade the Homebrew package, run
```
brew services stop spacebar
brew upgrade spacebar
brew services start spacebar
```

If you're using the Nix package and keeping your channels up to date, package upgrades will roll in as you command.

## Requirements and Caveats
Please read the below requirements carefully.  
Make sure you fulfil all of them before filing an issue.

|Requirement|Note|
|-:|:-|
|Operating System|macOS Catalina 10.15.0+ is supported.|
|Accessibility API|spacebar must be given permission to utilize the Accessibility API and will request access upon launch. The application must be restarted after access has been granted.|

Please also take note of the following caveats.

|Caveat|Note|
|-:|:-|
|Code Signing|When building from source (or installing from HEAD), it is recommended to codesign the binary so it retains its accessibility and automation privileges when updated or rebuilt.|
|Mission Control|In the Mission Control preferences pane in System Preferences, the setting "Automatically rearrange Spaces based on most recent use" should be disabled.|

## Releases and branches
Main work for this project is conducted on the `master` branch, and thus it should be considered unstable (expect bugs!).  
There is no particular release cycle, just as and when features/fixes are ready :)

## License and Attributions
spacebar is licensed under the [MIT License](LICENSE.txt), a short and simple permissive license with conditions only requiring preservation of copyright and license notices.
Licensed works, modifications, and larger works may be distributed under different terms and without source code.

Many thanks to [@koekeishiya](https://github.com/koekeishiya) for creating yabai, and providing the codebase for an example status bar, from which this project was born.

## Disclaimer
Use at your own discretion.  
I take no responsibility if anything should happen to your machine while trying to install, test or otherwise use this software in any form.
