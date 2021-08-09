#!/usr/bin/env bash
RUNNING=$(osascript -e 'if application "Mail" is running then return 0')
COUNT=0

if [ $RUNNING == 0 ]; then
  COUNT=$(osascript -e 'if application "Mail" is running then tell application "Mail" to return the unread count of inbox')
fi
if [ $RUNNING == 0 ] && [ $COUNT -gt 0 ]; then
  echo "  $COUNT | "
else
  echo " "
fi

