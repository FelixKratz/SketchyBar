#!/usr/bin/env bash
MAX_DOWN=110
MAX_UP=45
UPDOWN=$(ifstat -i "en0" -b 0.1 1 | tail -n1)
sketchybar -m push network_down $(echo $UPDOWN | awk "{ print \$1 / (1000.0 * $MAX_DOWN) }")
sketchybar -m push network_up $(echo $UPDOWN | awk "{ print \$2 / (1000.0 * $MAX_UP) }")
