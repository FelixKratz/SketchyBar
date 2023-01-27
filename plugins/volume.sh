#!/usr/bin/env sh

# The volume_change event supplies a $INFO variable in which the current volume
# percentage is passed to the script. When sketchybar starts this script is
# forced to run via "sketchybar --update" at the end of the configuration file,
# to populate the volume before an actual volume_change event.
# The $INFO variable will thus be unset for this first execution of the script
# and we fall back to get the current volume via Apple Script in this case.

VOLUME=${INFO:-"$(osascript -e "output volume of (get volume settings)")"}

case ${VOLUME} in
  [6-9][0-9]|100) ICON="墳"
  ;;
  [3-5][0-9]) ICON="奔"
  ;;
  [1-9]|[1-2][0-9]) ICON="奄"
  ;;
  *) ICON="婢"
esac

# The item invoking this script (name $NAME) will get its icon and label
# updated with the current battery status
sketchybar --set $NAME icon="$ICON" label="${VOLUME}%"
