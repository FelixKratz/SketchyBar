#!/usr/bin/env bash
MAX_DOWN=110
sketchybar -m push network_down 0 $(ifstat -i "en0" -b 0.1 1 | tail -n1 | awk "{ print \$1 / (1000.0 * $MAX_DOWN) }")
