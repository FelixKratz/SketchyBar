# v1.4.x -> v2.0.0
This document highlights the steps needed to update an existing sketchybar configuration to v2.0.0.

## Command Structure
* The *batch* command does not exist anymore, you can simply remove all occurences of *batch* in your configuration and all should still work.
* The *non-batch* version of all commands has been removed and superseded. Now *all* commands are prefixed with *--*
and contain *<key>=<value>* pairs. If you have used the batch syntax exclusively, you do not need to change anything, except removing all occurences of *batch*.

* The keyword *component* has been trimmed, simply delete all occurences of *component* in the configuration.

## Renamed Properties
*--set*:
* *icon_font* -> *icon.font*
* *icon_color* -> *icon.color*
* *icon_highlight_color* -> *icon.highlight_color*
* *icon_padding_left* -> *icon.padding_left*
* *icon_padding_right* -> *icon.padding_right*
* *icon_highlight* -> *icon.highlight*
* *label_font* -> *label.font*
* *label_color* -> *label.color* 
* *label_highlight_color* -> *label.highlight_color*
* *label_padding_left* -> *label.padding_left*
* *label_padding_right* -> *label.padding_right*
* *label_highlight* -> *label.highlight*
* *draws_background* -> *background.drawing*
* *background_color* -> *background.color*
* *background_height* -> *background.height*
* *background_border_color* -> *background.border_color*
* *background_corner_radius* -> *background.corner_radius*
* *background_border_width* -> *background.border_width*
* *background_padding_left* -> *background.padding_left*
* *background_padding_right* -> *background.padding_right*
* *graph_color* -> *graph.color*
* *graph_fill_color* -> *graph.fill_color*
* *graph_line_width* -> *graph.line_width*

Easy renaming: simply find and replace the follwing strings 
* *icon_* -> *icon.*
* *label_* -> *label.*
* *background_* -> *background.*
* *graph_* -> *graph.*
* *draws_background* -> *background.drawing*

*--bar*:
* *bar_color* -> *color*

## Removed Modifiers
* *nospace* modifier has been removed in favour of the *width* property. Set *width=0* for equivalent behaviour

## Removed/Renamed Domains
* *config* -> *--bar*
* *set* -> *--set*
* *add* -> *--add*
* *query* -> *--query*
* *update* -> *--update*
* *push* -> *--push*
* *trigger* -> *--trigger*
* *subscribe* -> *--subscribe*
* *default* -> *--default*
* *freeze*
* *remove*

## Replaced Properites
* *scripting*, replaced with *updates*
* *enabled*, replaced with *updates* and *drawing*

