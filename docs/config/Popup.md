---
id: popups
title: Popup Menus
sidebar_position: 1
---
## Popup Menus
![Simple Popup](https://user-images.githubusercontent.com/22680421/146688291-b8bc5e77-e6a2-42ee-bd9f-b3709c63d936.png)

Popup menus are a powerful way to make further `items` accessible in a small popup window below any bar item.
Every item has a popup available with the properties:

| <property\>                              | <value\>                  | default    | description                                                  |
| :-------:                                | :------:                  | :-------:  | -----------                                                  |
| `popup.drawing`                          | `<boolean>`               | `off`      | If the `popup` should be rendered                            |
| `popup.horizontal`                       | `<boolean>`               | `off`      | If the `popup` should render horizontally                    |
| `popup.height`                           | `<positive_integer>`      | bar height | The vertical spacing between items in a popup (Only on HEAD) |
| `popup.y_offset`                         | `<integer>`               | `0`        | Vertical offset applied to the `popup`                       |
| `popup.align`                            | `left`, `right`, `center` | `left`     | Alignment of the popup with its parent item in the bar       |
| `popup.background.<background_property>` |                           |            | Popups have a background and support all properties          |

Items can be added to a popup menu by setting the `position` of those items to `popup.<name>` where `<name>` is the name of the item containing the popup.
You can find a demo implementation of this [here](https://github.com/FelixKratz/SketchyBar/discussions/12?sort=new#discussioncomment-1843975).

