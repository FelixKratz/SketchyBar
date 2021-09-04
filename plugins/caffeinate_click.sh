#!/usr/bin/env bash

if [ "$(pgrep -x "caffeinate")" = "" ]; then
  caffeinate &disown;
  sketchybar -m set $NAME icon 
else
  killall caffeinate
  sketchybar -m set $NAME icon ﯈
fi
