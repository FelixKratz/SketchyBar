#!/usr/bin/env bash
MAX_DOWN=110000
MAX_UP=45000
UPDOWN=$(ifstat -i "en0" -b 0.1 1 | tail -n1)
DOWN=$(echo $UPDOWN | awk "{ print \$1 }" | cut -f1 -d ".")
UP=$(echo $UPDOWN | awk "{ print \$2 }" | cut -f1 -d ".")

sketchybar -m push network_down $(echo $UPDOWN | awk "{ print \$1 / (1.0 * $MAX_DOWN) }")
sketchybar -m push network_up $(echo $UPDOWN | awk "{ print \$2 / (1.0 * $MAX_UP) }")

DOWN_STR=""
UP_STR=""
if [ "$DOWN" -lt "1000" ]; then
  DOWN_STR="$DOWN kbps"
else
  DOWN_STR=$(echo "scale=2; $DOWN" | bc -l)" Mbps"
fi
if [ "$UP" -lt "1000" ]; then
  UP_STR="$UP kbps"
else
  UP_STR=$(echo "scale=2; $UP" | bc -l)" Mbps"
fi
sketchybar -m set network_label label " $DOWN_STR 祝 $UP_STR"
