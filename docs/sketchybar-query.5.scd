sketchybar-query(5)

# NAME

sketchybar-query - query information about bar, items, events, etc.

# SYNOPSIS

*sketchybar --query* *bar*|_NAME_|*defaults*|*events*|*default_menu_items*

# DESCRIPTION

Sketchybar can be queried for information about a number of things.

# OPTIONS

*--query bar*
	Output a json structure with info about the bar.

*--query* _NAME_
	Output a json structure with info about a specific item referred to by _NAME_.

*--query defaults*
	Output a json structure with info about the current defaults.

*--query events*
	Output a json structure with info about events.

*--query default_menu_items*
	Output a json structure with the names of the menu bar items in the
	default macOS bar. This is useful for configuring aliases (see
	*sketchybar-components*(5)).

# SEE ALSO

*sketchybar*(1)++
*sketchybar*(5)++
*sketchybar-items*(5)++
*sketchybar-components*(5)++
*sketchybar-popup*(5)++
*sketchybar-events*(5)++
*sketchybar-animate*(5)++
*sketchybar-types*(5)++
*sketchybar-tips*(5)
