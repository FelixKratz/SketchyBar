#!/usr/bin/env bash

NOTIFICATIONS="$(gh api notifications | jq '.[]')"

if [ "$NOTIFICATIONS" = "" ]; then
  sketchybar -m --set $NAME icon=
else
  sketchybar -m --set $NAME icon=
fi

