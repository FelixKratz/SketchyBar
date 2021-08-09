#!/usr/bin/env bash

COUNT=$(curl https://github.com/users/FelixKratz/contributions | grep $(date '+%Y-%m-%d') | sed -nr 's/.*data-count=\"([^"]+).*/\1/p')

if [ $COUNT -gt 0 ]; then
  spacebar -m set githubIndicator icon_color 0xaa48aa2a
else
  spacebar -m set githubIndicator icon_color 0xaaffffff
fi
