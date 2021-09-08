#!/usr/bin/env bash

if [ "$SELECTED" = "true" ]; then
  sketchybar -m batch --set $NAME label_highlight=on icon_highlight=on
else
  sketchybar -m batch --set $NAME label_highlight=off icon_highlight=off
fi
