#!/usr/bin/env bash

if [ "$SELECTED" = "true" ]; then
  sketchybar -m freeze on
  sketchybar -m set $NAME icon_highlight on
  sketchybar -m set $NAME label_highlight on
  sketchybar -m freeze off
else
  sketchybar -m freeze on
  sketchybar -m set $NAME icon_highlight off
  sketchybar -m set $NAME label_highlight off
  sketchybar -m freeze off
fi
