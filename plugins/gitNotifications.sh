#!/usr/bin/env bash

NOTIFICATIONS="$(gh api notifications | jq '.[]')"

if [ "$NOTIFICATIONS" = "" ]; then
  sketchybar --set $NAME icon=
else
  sketchybar --set $NAME icon=
fi

