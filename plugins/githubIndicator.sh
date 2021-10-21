#!/usr/bin/env bash

COUNT=0
COUNT=$(curl https://github.com/users/FelixKratz/contributions | grep $(date '+%Y-%m-%d') | sed -nr 's/.*data-count=\"([^"]+).*/\1/p')

echo $COUNT
if [ $COUNT -gt 0 ]; then
  sketchybar -m --set $NAME icon.color=0xff48aa2a label.padding_left=4 label="$COUNT"
else
  sketchybar -m --set $NAME icon.color=0xaaffffff label.padding_left=0 label=""
fi
