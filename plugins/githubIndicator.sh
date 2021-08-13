#!/usr/bin/env bash

COUNT=0
COUNT=$(curl https://github.com/users/FelixKratz/contributions | grep $(date '+%Y-%m-%d') | sed -nr 's/.*data-count=\"([^"]+).*/\1/p')

echo $COUNT
if [ $COUNT -gt 0 ]; then
  sketchybar -m set githubIndicator icon_color 0xff48aa2a
  sketchybar -m set githubIndicator label_padding_left 4
  sketchybar -m set githubIndicator label $COUNT
else
  sketchybar -m set githubIndicator icon_color 0xaaffffff
  sketchybar -m set githubIndicator label_padding_left 0
  sketchybar -m set githubIndicator label ""
fi
