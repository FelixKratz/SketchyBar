#!/usr/bin/env bash

COUNT=0
COUNT=$(curl https://github.com/users/FelixKratz/contributions | grep $(date '+%Y-%m-%d') | sed -nr 's/.*data-count=\"([^"]+).*/\1/p')

echo $COUNT
if [ $COUNT -gt 0 ]; then
  sketchybar -m set $NAME icon_color 0xff48aa2a
  sketchybar -m set $NAME label_padding_left 4
  sketchybar -m set $NAME label $COUNT
else
  sketchybar -m set $NAME icon_color 0xaaffffff
  sketchybar -m set $NAME label_padding_left 0
  sketchybar -m set $NAME label ""
fi
