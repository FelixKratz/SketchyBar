---
id: querying
title: Querying Information
sidebar_position: 1
---
## Querying
*SketchyBar* can be queried for information about a number of things.
### Bar Properties
Information about the bar can be queried via:
```bash
sketchybar --query bar
```
The output is a JSON structure containing relevant information about the configuration settings of the bar.
### Item Properties
Information about an item can be queried via:
```bash
sketchybar --query <name>
```
The output is a JSON structure containing relevant information about the configuration of the item.
### Default Properties
Information about the current defaults.
```bash
sketchybar --query defaults
```
### Event Properties
Information about the events.
```bash
sketchybar --query events
```

### macOS Menu Bar Item Names (for use with aliases)
The names of the menu bar items in the default macOS bar:
```bash
sketchybar --query default_menu_items
```

### Display Configuration Information
The names of the menu bar items in the default macOS bar:
```bash
sketchybar --query displays
```
