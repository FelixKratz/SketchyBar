#!/usr/bin/env bash

make install
ln ./bin/sketchybar /usr/local/bin/sketchybar
mkdir ~/.config/sketchybar
cp sketchybarrc ~/.config/sketchybar/sketchybarrc
cp -r plugins ~/.config/sketchybar
chmod +x ~/.config/sketchybar/sketchybarrc
chmod +x ~/.config/sketchybar/plugins/*

echo "Install complete..."
