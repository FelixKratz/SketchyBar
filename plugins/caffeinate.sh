#!/usr/bin/env bash

if [ "$(pgrep -x "caffeinate")" = "" ]; then
  sketchybar -m set $NAME icon ﯈
else
  sketchybar -m set $NAME icon 
fi
