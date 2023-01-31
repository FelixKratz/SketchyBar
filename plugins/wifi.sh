#!/bin/sh

# The wifi_change event supplies a $INFO variable in which the current SSID
# is passed to the script.

WIFI=${INFO:-"Not Connected"}

sketchybar --set $NAME label="${WIFI}"
