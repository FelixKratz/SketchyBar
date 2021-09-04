#!/usr/bin/env bash

if [ "$SELECTED" = "true" ]; then
  sketchybar -m set $NAME icon_highlight on
  sketchybar -m set $NAME label_highlight on
else
  sketchybar -m set $NAME icon_highlight off
  sketchybar -m set $NAME label_highlight off
fi
