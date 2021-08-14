#!/usr/bin/env bash
MAX_UP=45
sketchybar -m push network_up 0 $(ifstat -i "en0" -b 0.1 1 | tail -n1 | awk "{ print \$2 / (1000.0 * $MAX_UP) }")
