#!/usr/bin/env bash
RUNNING=$(osascript -e 'if application "Spotify" is running then return 0')
if [ "$RUNNING" == "" ]; then
  RUNNING=1
fi
PLAYING=1
TRACK=""
ALBUM=""
ARTIST=""
if [ "$(osascript -e 'if application "Spotify" is running then tell application "Spotify" to get player state')" == "playing" ]; then
  PLAYING=0
  TRACK=$(osascript -e 'tell application "Spotify" to get name of current track')
  ARTIST=$(osascript -e 'tell application "Spotify" to get artist of current track')
  ALBUM=$(osascript -e 'tell application "Spotify" to get album of current track')
fi
if [ $RUNNING -eq 0 ] && [ $PLAYING -eq 0 ]; then
  if [ "$ARTIST" == "" ]; then
    sketchybar -m --set $NAME label=" $TRACK  $ALBUM"
  else
    sketchybar -m --set $NAME label=" $TRACK  $ARTIST"
  fi
else
  sketchybar -m --set $NAME label=""
fi
