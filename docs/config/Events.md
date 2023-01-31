---
id: events
title: Events & Scripting
sidebar_position: 1
---
## Events and Scripting
All items can *subscribe* to arbitrary *events*; when the *event* happens,
all items subscribed to the *event* will execute their *script*.
This can be used to create more reactive and performant items which react to
events rather than polling for a change.
```bash
sketchybar --subscribe <name> <event> ... <event>
```
where the events are:

| <event\>               | description                                                                                         | `$INFO`                                |
| :-------:              | :------:                                                                                            | :------:                               |
| `front_app_switched`   | When the front application changes (not triggered if a different window of the same app is focused) | front application name                 |
| `space_change`         | When the active mission control space changes                                                       | JSON for active spaces on all displays |
| `display_change`       | When the active display is changed                                                                  | new active display id                  |
| `volume_change`        | When the system audio volume is changed                                                             | new volume in percent                  |
| `brightness_change`    | When a displays brightness is changed                                                               | new brightness in percent              |
| `power_source_change`  | When the devices power source is changed                                                            | new power source (`AC` or `BATTERY`)   |
| `wifi_change`          | When the device connects of disconnects from wifi                                                   | new WiFi SSID or empty on disconnect   |
| `system_will_sleep`    | When the system prepares to sleep                                                                   |                                        |
| `system_woke`          | When the system has awaken from sleep                                                               |                                        |
| `mouse.entered`        | When the mouse enters over an item                                                                  |                                        |
| `mouse.exited`         | When the mouse leaves an item                                                                       |                                        |
| `mouse.entered.global` | When the mouse enters over *any* part of the bar                                                    |                                        |
| `mouse.exited.global`  | When the mouse leaves *all* parts of the bar                                                        |                                        |
| `mouse.clicked`        | When an item is clicked                                                                             |                                        |

Some events send additional information in the `$INFO` variable
When an item is subscribed to these events the *script* is run and it gets passed the `$SENDER` variable, which holds exactly the above names to distinguish between the different events.
It is thus possible to have a script that reacts to each event differently e.g. via a switch for the `$SENDER` variable in the *script*.

Alternatively a fixed *update_freq* can be *--set*, such that the event is routinely run to poll for change, the `$SENDER` variable will in this case hold the value `routine`.

When an item invokes a script, the script has access to some environment variables, such as:
```bash
$NAME
$SENDER
```
Where `$NAME` is the name of the item that has invoked the script and `$SENDER` is the reason why the script is executed.

If an item is *clicked* the script has access to the additional variables:
```bash 
$BUTTON
$MODIFIER
```
where the `$BUTTON` can be *left*, *right* or *other* and specifies the mouse button that was used to click the item, while the `$MODIFIER` is either *shift*, *ctrl*, *alt* or *cmd* and 
specifies the modifier key held down while clicking the item.


All scripts are forced to terminate after 60 seconds and do not run while the system is sleeping. 

### Creating custom events
This allows to define events which are triggered by arbitrary applications or manually (see Trigger custom events).
Items can also subscribe to these events for their script execution.
```bash
sketchybar --add event <name> [optional: <NSDistributedNotificationName>]
```
Optional: You can subscribe to the notifications sent to the NSDistributedNotificationCenter e.g.
the notification Spotify sends on track change:
`com.spotify.client.PlaybackStateChanged` ([example](https://github.com/FelixKratz/SketchyBar/discussions/12#discussioncomment-1455842)), or the
notification sent by the system when the screen is unlocked:
`com.apple.screenIsUnlocked` ([example](https://github.com/FelixKratz/SketchyBar/discussions/12?sort=new#discussioncomment-2979651))
to create more responsive items.
Custom events that subscribe to NSDistributedNotificationCenter notifications
will receive additional notification information in the `$INFO` variable if available.
For more NSDistributedNotifications see [this discussion](https://github.com/FelixKratz/SketchyBar/discussions/151).

### Triggering custom events
This triggers a custom event that has been added before
```bash
sketchybar --trigger <event> [Optional: <envvar>=<value> ... <envvar>=<value>]
```
Optionally you can add environment variables to the trigger command witch are passed to the script, e.g.:
```bash
sketchybar --trigger demo VAR=Test
```
will trigger the demo event and `$VAR` will be available as an environment variable in the scripts that this event invokes.

### Forcing all shell scripts to run and the bar to refresh
This command forces all scripts to run and all events to be emitted, it should
*never* be used in an item script, as this would lead to infinite loops. It
is prominently needed after the initial configuration to properly initialize
all items by forcing all their scripts to run
```bash
sketchybar --update
```

