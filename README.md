# SketchyBar
This is a rewrite of the spacebar project, which itself is a rewrite of the statusbar code from yabai.
What I have added:
* As many widgets as you like at any of the three positions: left, center, right
* The order of the widgets in the sketchybarrc file will be the order in which they show in the bar
* Associate widgets to certain displays or spaces, to show specific information on the relevant screens/displays
* The widgets are highly customizable with settings for different fonts, colors, icon paddings, label paddings, etc. for each individual element
* Relocate *all* visible components of the bar (even spaces or the window title, etc.)
* Draw arbitrary graphs in the bar with external data provider scripts that push the data into the graph
* Overlay as many graphs as wanted, like system cpu usage and user cpu usage in one figure
* Individual refresh frequencies for each widget
* ... feel free to explore my sketchybarrc file for more details on the options

I have many more plans for the project:
* Let items subscribe to system events (e.g. space changed, window focused, etc.) for their refresh action (like in yabai)
* Cache the scripts in RAM to reduce I/O operations
* Make the associated_space / associated_display properties more powerful by allowing to associate to more than one screen/display
* Make application specific widgets with associated_app argument (e.g. when gvim is open show the vim mode indicator in the status bar)
* Fix the currently static positioning of the bar
* A y_offset property for all items to create (in combination with the nospace modifier) vertically stacked labels
* Add on_click events for the widgets
* Create more plugins
* ......

This is my setup:
![](images/mySetup.png)
where I have my screens and a vim mode indicator on the left. Not shown is the high memory warning which shows the process that is using high system memory on demand.
In the center I have a spotify indicator (only when music is playing) and on the right I have (not shown) a high cpu process indicator, as well as a cpu graph, a github contribution counter, a new mail counter and the current date.

The cpu and memory indicators are only shown on the "code" screen and are not visible on the other screens.

## Installation
Clone the repo and run 
```bash
make install
ln ./bin/sketchybar /usr/local/bin/sketchybar
```
Now you can create you configuration inside of $HOME/.config/sketchybar/sketchybarrc
and finally run the bar via
```bash
sketchybar
```
If you want to use plugins, make sure that they are referenced in the rc with the correct path and that they are made executable via
```bash
chmod +x name/of/plugin.sh
```
You should of course vet the code from all plugins before granting them the executable bit to make sure they are not harming your computer.


Credits:
yabai,
spacebar,
reddit,
many more for the great code base and inspiration
