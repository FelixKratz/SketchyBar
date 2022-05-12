---
id: features
title: Features
sidebar_position: 1
---

## Features

* Performance friendly
* No accessibility permissions needed
* Fully scriptable
* Fully configurable (Fonts, Backgrounds, Colors, Icons, etc.)
* Supports drawing native macOS menu bar applications (aliases)
* Powerful event and scripting system
* Popup Menus
* Mouse Support
* Support for graphs
* Per display and per space individualization

The configuration of the bar takes place in a configuration file where almost everything can be configured.
Basically, the bar itself is a rectangle that can hold arbitrarily many *items*, which can be configured to do awesome stuff.
An *item* will occupy a space in the bar and can be equipped to show an *icon* and a *label*. The *icon* and *label* can be changed through
*scripts* that can be attached to the *item*. It is also possible to *subscribe* an *item* to certain *events* for their *script* execution action,
which makes very powerful items possible.
Furthermore, an *item* can be assigned to mission control spaces or displays, such that they only show on a certain space or display, which makes multi-desktop configuration
of the bar possible and opens the possibility to create individualized bar configuration on a per display and per space level.
These simple ingredients make *items* almost endlessly customizable and can be used to display arbitrary information and perform useful actions. For some examples see my sketchybarrc and
the plugins folder.

Some special features can not be accomplished with a simple *item*, this is where the *components* come into play. They basically are *items* with
extra steps. They contain all the properties a regular item does, but they can do specialized tasks a simple item can not. For example, there
is a *graph* component, which can be used to display graphs in the bar.

For more details on how the configuration works, see the configuration section.

## Examples
![examples](/img/examples.png)
