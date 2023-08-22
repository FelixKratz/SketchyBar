#!/bin/sh

# The wifi_change event supplies a $INFO variable in which the current SSID
# is passed to the script.

if [ "$SENDER" = "wifi_change" ]; then
  WIFI=${INFO:-"Not Connected"}
  sketchybar --set $NAME label="${WIFI}"
fi
