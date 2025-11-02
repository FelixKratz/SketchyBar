sketchybar-events(5)

# NAME

sketchybar-events - configuration of events and scripting

# SYNOPSIS

*sketchybar --add event* _NAME_ [_NSDistributedNotificationName_] ++
*sketchybar --subscribe* _NAME_ _EVENT_ ... _EVENT_ ++
*sketchybar --trigger* _NAME_ [_ENVVAR_=_VALUE_ ... _ENVVAR_=_VALUE_] ++
*sketchybar --update*

# DESCRIPTION

All items can subscribe to events. When an event is triggered, all items
subscribed to this event will execute their scripts. This mechanism can be
used to create more reactive and performant items which react to events rather
than to poll changes.

Some events send additional information through the *$INFO* environment
variable. When a script is run due to an event, the *$SENDER* variable is passed.
This variable holds the event name (see EVENTS) and can be used to distinguish
between different events in a script. It is thus possible to have a script
that reacts differently to various events (consider: a switch statement via
shell's *case* command for the *$SENDER* variable in the *script*).

Alternatively, a fixed *update_freq* can be *--set* such that the event is
routinely run to poll for changes. Here, the *$SENDER* variable will hold the
value *routine*.

All scripts invoked by an item have access to some environment variables such as
- *$NAME*
- *$SENDER*
- *$CONFIG_DIR*
where *$NAME* is the name of the item that invoked the script and *$SENDER* is
the reason why the script was executed. *$CONFIG_DIR* holds the value of the
absolute path of the directory where the current configuration file is located.

If an item is *clicked*, the script being called (either *click_script* or the
*script* when subscribed to *mouse.clicked*) has access to the additional
variables such as
- *$BUTTON*
- *$MODIFIER*
where *$BUTTON* can have a value of *left*, *right* or *other*, and does
specify the mouse button that was used to click the item. Whereas, *$MODIFIER*
can be *shift*, *ctrl*, *alt* or *cmd*, and does specify the modifier key held
down while clicking the item.

If an item receives a *scroll* event from the mouse, the script sends the
additional *$SCROLL_DELTA* variable.

All scripts are *forced* to terminate after 60 seconds and do not run while
the system is sleeping.

# OPTIONS

*--add event* _NAME_ [_NSDistributedNotificationName_]
	Add an event to be triggered manually or by arbitrary applications.
	Optionally, an event can use the notifications sent to macOS's notification
	system by specifying the relevant _NSDistributedNotificationName_.

	For example, Spotify notifies the system on track change via
	*com.spotify.client.PlaybackStateChanged*, and the system notifies itself
	when the screen is unlocked via *com.apple.screenIsUnlocked*.

	Custom events that subscribe to *NSDistributedNotificationCenter*
	notifications will receive additional notification information through the
	*$INFO* variable if available.

	For more on *NSDistributedNotifications*, see 
	https://github.com/FelixKratz/SketchyBar/discussions/151

*--subscribe* _NAME_ _EVENT_ ... _EVENT_
	Subscribe an item referred by _NAME_ to an _EVENT_ or a series of events.
	A list of events can be found in EVENTS. See below.

*--trigger* _NAME_ [_ENVVAR_=_VALUE_ ... _ENVVAR_=_VALUE_]
	Trigger any custom event. Optionally, specify _ENVVAR_ with value _VALUE_ 
	(or a series of such variables) to pass them on to the *script* or 
	*click_script*.

*--update*
	Forces all *scripts* to run and all *events* to be emitted. This should 
	*NEVER* be used in an item script as this would lead to infinite loops. 
	But, this is prominently needed after the initial configuration to properly 
	initialize all the items by forcing all their scripts to run.

# EVENTS

[[ *event*
:[ *description*
:< *returned $INFO*
|  front_app_switched
:  When the front application changes (not triggered if a different window of
   the same app is focused)
:  front application name
|  space_change
:  When the active mission control space changes
:  list of active spaces on all displays in a JSON structure
|  space_windows_change
:  When a window is created or destroyed on a space
:  list of application windows on the respective space in a JSON structure
|  display_change
:  When the active display is changed
:  new active display id
|  volume_change
:  When the system audio volume is changed
:  new volume in percent
|  brightness_change
:  When a displays brightness is changed
:  new brightness in percent
|  power_source_change
:  When the devices power source is changed
:  new power source (AC or BATTERY)
|  wifi_change
:  When the device connects of disconnects from wifi
:  new WiFi SSID or empty on disconnect (not working since macOS Sonoma)
|  media_change
:  When a change in now playing media is performed (deprecated since macOS 26)
:  media info in a JSON structure
|  system_will_sleep
:  When the system prepares to sleep
:  -
|  system_woke
:  When the system has awaken from sleep
:  -
|  mouse.entered
:  When the mouse enters over an item
:  -
|  mouse.exited
:  When the mouse leaves an item
:  -
|  mouse.entered.global
:  When the mouse enters over *any* part of the bar
:  -
|  mouse.exited.global
:  When the mouse leaves *all* parts of the bar
:  -
|  mouse.clicked
:  When an item is clicked
:  mouse button and modifier info
|  mouse.scrolled
:  When the mouse is scrolled over an item
:  scroll wheel delta
|  mouse.scrolled.global
:  When the mouse is scrolled over an empty region of the bar
:  scroll wheel delta

# EXAMPLES

A simple plugin that uses the *com.spotify.client.PlaybackStateChanged*
notification sent by Spotify can be found at
https://github.com/FelixKratz/SketchyBar/discussions/12#discussioncomment-1455842

A more advanced plugin that uses the *com.apple.screenIsUnlocked* notification
sent by the system and the animation feature (see *sketchybar-animate*(5)) can
be found at
https://github.com/FelixKratz/SketchyBar/discussions/12?sort=new#discussioncomment-2979651

# SEE ALSO

*sketchybar*(1) ++
*sketchybar*(5) ++
*sketchybar-items*(5) ++
*sketchybar-components*(5) ++
*sketchybar-popup*(5) ++
*sketchybar-query*(5) ++
*sketchybar-animate*(5) ++
*sketchybar-types*(5) ++
*sketchybar-tips*(5)
