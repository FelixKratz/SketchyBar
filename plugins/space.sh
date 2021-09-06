#!/usr/bin/env bash

echo "Ich heiße: $NAME und bin $SELECTED";

if [ "$SELECTED" = "true" ]; then
  sketchybar -m batch --set $NAME label_highlight=on icon_highlight=on
else
  sketchybar -m batch --set $NAME label_highlight=off icon_highlight=off
fi
