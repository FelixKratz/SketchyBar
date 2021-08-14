#!/usr/bin/env bash
MAX_DOWN=110
MAX_UP=45
UPDOWN=$(ifstat -i "en0" -b 0.1 1 | tail -n1)
sketchybar -m push network_down 0 $(echo $UPDOWN | awk "{ print \$1 / (1000.0 * $MAX_DOWN) }")
sketchybar -m push network_up 0 $(echo $UPDOWN | awk "{ print \$2 / (1000.0 * $MAX_UP) }")
