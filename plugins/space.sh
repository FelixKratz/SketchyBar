#!/usr/bin/env bash

if [ "$SELECTED" = "true" ]; then
  sketchybar -m --set $NAME label.highlight=on icon.highlight=on
else
  sketchybar -m --set $NAME label.highlight=off icon.highlight=off
fi
