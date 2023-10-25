#!/bin/sh

rm -rf build
mkdir build
scdoc < sketchybar-animate.5.scd > build/sketchybar-animate.5
scdoc < sketchybar-components.5.scd > build/sketchybar-components.5
scdoc < sketchybar-events.5.scd > build/sketchybar-events.5
scdoc < sketchybar-items.5.scd > build/sketchybar-items.5
scdoc < sketchybar-popup.5.scd > build/sketchybar-popup.5
scdoc < sketchybar-query.5.scd > build/sketchybar-query.5
scdoc < sketchybar-tips.5.scd > build/sketchybar-tips.5
scdoc < sketchybar-types.5.scd > build/sketchybar-types.5
scdoc < sketchybar.1.scd > build/sketchybar.1
scdoc < sketchybar.5.scd > build/sketchybar.5

tar -czvf documentation.tar.gz -C build .
rm -rf build
