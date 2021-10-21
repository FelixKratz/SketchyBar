#!/usr/bin/env bash
MAX_DOWN=110000
MAX_UP=45000
UPDOWN=$(ifstat -i "en0" -b 0.1 1 | tail -n1)
DOWN=$(echo $UPDOWN | awk "{ print \$1 }" | cut -f1 -d ".")
UP=$(echo $UPDOWN | awk "{ print \$2 }" | cut -f1 -d ".")

sketchybar -m --push network_down $(echo $UPDOWN | awk "{ print \$1 / (1.0 * $MAX_DOWN) }")
              --push network_up $(echo $UPDOWN | awk "{ print \$2 / (1.0 * $MAX_UP) }")
