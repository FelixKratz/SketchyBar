sketchybar(1)

# NAME

sketchybar - a highly customizable macOS status bar replacement

# SYNOPSIS

*sketchybar* ++
*sketchybar --config* _FILE_ ++
*sketchybar --reload* [_CONFIG_FILE_] ++
*sketchybar --hotload* _BOOLEAN_

*sketchybar --bar* _SETTING_=_VALUE_ ... _SETTING_=_VALUE_

*sketchybar --add item* _NAME_ _POSITION_ ++
*sketchybar --set* _NAME_ _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_ ++
*sketchybar --default* _PROPERTY_=_VALUE_ ... _PROPERTY_=_VALUE_ ++
*sketchybar --reorder* _NAME_ ... _NAME_ ++
*sketchybar --move* _NAME_ *before*|*after* _REFERENCE_NAME_ ++
*sketchybar --clone* _PARENT_NAME_ _NAME_ [*before*|*after*] ++
*sketchybar --rename* _OLD_NAME_ _NEW_NAME_ ++
*sketchybar --remove* _NAME_

*sketchybar --add graph* _NAME_ _POSITION_ _WIDTH_ *--push* _NAME_
_DATA_POINT_ ... _DATA_POINT_ ++
*sketchybar --add space* _NAME_ _POSITION_ ++
*sketchybar --add bracket* _NAME_ _MEMBER_NAME_ ... _MEMBER_NAME_ ++
*sketchybar --add alias* _APPLICATION_NAME_|"_WINDOW_OWNER_,_WINDOW_NAME_" _POSITION_ ++
*sketchybar --add slider* _NAME_ _POSITION_ _WIDTH_

*sketchybar --set* _NAME_ *popup*._PROPERTY_=_VALUE_

*sketchybar --add event* _NAME_ [_NSDistributedNotificationName_] ++
*sketchybar --subscribe* _NAME_ _EVENT_ ... _EVENT_ ++
*sketchybar --trigger* _NAME_ [_ENVVAR_=_VALUE_ ... _ENVVAR_=_VALUE_] ++
*sketchybar --update*

*sketchybar --query* *bar*|_NAME_|*defaults*|*events*|*default_menu_items*

*sketchybar --animate* _CURVE_ _DURATION_ *--bar* _SETTING_=_VALUE_ ...
_SETTING_=_VALUE_ ++
*sketchybar --animate* _CURVE_ _DURATION_ *--set* _PROPERTY_=_VALUE_ ...
_PROPERTY_=_VALUE_

# OPTIONS

*--config* _FILE_
	Specify an alternative configuration file. The default one found at
	_$HOME/.config/sketchybar/sketchybarrc_ is automatically read when sketchybar
	is launched.

*--reload* [_CONFIG_FILE_]
	Reload the bar without manually restarting the process. Essentially, this
	has the same effect as restarting the process, but is a bit more convenient.
	Additionally, an optional path to a new _CONFIG_FILE_ can be given to load
	a different configuration. If the optional argument is left out, the current
	configuration is reloaded.

*--hotload* _BOOLEAN_
	Enables the hotload feature. If you wish that the bar automatically reloads
	the configuration file once it is edited, the hotload functionality can be
	used. It will monitor the directory of the current configuration for changes
	and reload the configuration should it detect file changes.

For the other commands, see CONFIGURATION below for the relevant man page.

# DESCRIPTION

*sketchybar* aims to create a highly flexible, customizable, fast and powerful
status bar replacement for people that like playing with shell scripts.

Its features include
- performance friendly
- fully scriptable
- fully configurable (fonts, backgrounds, colors, icons, etc.)
- support for drawing native macOS menu bar applications (aliases)
- powerful event and scripting system
- animation system
- popup menus
- mouse support
- support for graphs
- per display and per space individualization.

The _bar_ is a rectangle that can hold arbitrarily many items which can be
configured to do awesome stuff. An _item_ will occupy a space in the bar and can
be equipped to show an icon and a label. The _icon_ and _label_ can be changed
through _scripts_ that can be attached to the item. It is also possible to
_subscribe_ an item to certain events for their script execution action, which
makes very powerful items possible.

Furthermore, an item can be assigned to mission control spaces or displays,
such that they only show on a certain space or display. This makes
multi-desktop configuration of the bar possible. This also opens the possibility
to create individualized bar configuration on a per display and per space level.

These simple ingredients make items almost endlessly customizable and can
be used to display arbitrary information and perform useful actions. Some
special features can not be accomplished with a simple item. This is where the
_components_ come into play which basically are items with extra steps. They
contain all the properties a regular item does, but they can do specialized
tasks a simple item can not. For example, there is a graph component which can
be used to display graphs in the bar.

For more details on how the configuration works, see CONFIGURATION.

# CONFIGURATION

A sketchybar configuration file is a regular shell script file that resides
at _$HOME/.config/sketchybar/sketchybarrc_. It is executed when sketchybar
launches. Everything persistent should be set up in this file.

It is possible to play with properties in the command line and change them
on the fly while the bar is running. Once a fitting value is found, the
configuration can then be included in the configuration file. The configuration
is then restored on restart.

When configuring the bar, it can be helpful to *stop the brew service* and *run
sketchybar* from the command line directly to see all relevant error messages
and warnings directly.

Refer to
- *sketchybar*(1) (this man page) for the overview of sketchybar
- *sketchybar*(5) to configure the bar
- *sketchybar-items*(5) to configure items
- *sketchybar-components*(5) to configure special components
- *sketchybar-popup*(5) to configure popup menus
- *sketchybar-events*(5) to configure events and scripting
- *sketchybar-query*(5) to query information
- *sketchybar-animate*(5) to animate
- *sketchybar-types*(5) for the nomenclature of the value types used in sketchybar
- *sketchybar-tips*(5) for configuration tips and tricks.

# SETUP

## Running sketchybar

Run the bar automatically at startup:
```
brew services start sketchybar
```

or in the command line with verbose output:
```
sketchybar
```

## Fonts

The default sketchybar font is the Hack Nerd Font. To install:
```
brew tap homebrew/cask-fonts
brew install --cask font-hack-nerd-font
```

If there exists missing icons, you might need to install it. Any font of your
liking can be used in sketchybar.

## Plugins

When additional plugins are used or created, make sure that they are made
executable via
```
chmod +x name_of_plugin.sh
```

If sketchybar is run from the command line directly with the command
*sketchybar*, all outputs and error messages from scripts will be printed in
stdout.

The default plugin folder is located at _$HOME/.config/sketchybar/plugins_.
Plugins need to be referenced with absolute paths because relative paths will
not be resolved correctly.

Have a look at the discussion for plugins and share your own if you want to.
Just make sure to vet the code from all plugins before executing them to make
sure they are not harming your computer.

## Hiding the system menu bar

To hide the original macOS bar in
- pre-Ventura: go to _System Preferences_ > _Dock & Menu Bar_, then check the
	  _Automatically hide and show the menu bar_ option
- Ventura: go to _System Settings_ > _Desktop & Dock_, then set the 
	  _Automatically hide and show the menu bar_ option to _Always_
- Sonoma: go to _System Settings_ > _Control Center_, then set the 
	  _Automatically hide and show the menu bar_ option to _Always_.

# SEE ALSO

*sketchybar*(5)++
*sketchybar-items*(5)++
*sketchybar-components*(5)++
*sketchybar-popup*(5)++
*sketchybar-events*(5)++
*sketchybar-query*(5)++
*sketchybar-animate*(5)++
*sketchybar-types*(5)++
*sketchybar-tips*(5)

# BUGS

Report them at https://github.com/FelixKratz/Sketchybar/issues.

# CREDITS

*Sketchybar* was forked from *spacebar* (see https://github.com/cmacrae/spacebar)
and completely reimagined and rewritten. The original idea is based on the
status bar that was included in *yabai* (see 
https://github.com/koekeishiya/yabai) before getting removed.

# MAINTAINERS

Felix Kratz <felix.kratz@tu-dortmund.de>
