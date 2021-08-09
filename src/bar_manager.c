#include "bar_manager.h"

int bar_manager_get_item_index_for_name(struct bar_manager* bar_manager, char* name) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    printf("Name: %s with requested name: %s \n", bar_manager->bar_items[i]->name, name);
    if (strcmp(bar_manager->bar_items[i]->name, name) == 0) {
      return i;
    }
  }
  return -1;
}

void bar_manager_set_foreground_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->foreground_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_background_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->background_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_space_icon_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->space_icon_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_position(struct bar_manager *bar_manager, char *pos)
{
  bar_manager->position = pos;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_title(struct bar_manager *bar_manager, bool value)
{
  bar_manager->title = value;
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_height(struct bar_manager *bar_manager, uint32_t height)
{
  bar_manager->height = height;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_padding_left(struct bar_manager *bar_manager, uint32_t padding)
{
  bar_manager->padding_left = padding;
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_padding_right(struct bar_manager *bar_manager, uint32_t padding)
{
  bar_manager->padding_right = padding;
  bar_manager_refresh(bar_manager);
}

void bar_manager_display_changed(struct bar_manager *bar_manager)
{
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_destroy(bar_manager->bars[i]);

  bar_manager_begin(bar_manager);
}

void bar_manager_set_display(struct bar_manager *bar_manager, char *display)
{
  bar_manager->display = display;

  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_destroy(bar_manager->bars[i]);

  bar_manager_begin(bar_manager);
}

void bar_manager_refresh(struct bar_manager *bar_manager)
{
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_refresh(bar_manager->bars[i]);
}

void bar_manager_resize(struct bar_manager *bar_manager)
{
  for (int i = 0; i < bar_manager->bar_count; ++i)
    bar_resize(bar_manager->bars[i]);
}

struct bar_item* bar_manager_create_item(struct bar_manager* bar_manager) {
    bar_manager->bar_items = (struct bar_item**) realloc(bar_manager->bar_items, sizeof(struct bar_item*) * (bar_manager->bar_item_count + 1));
    bar_manager->bar_item_count += 1;
    struct bar_item* bar_item = bar_item_create();
    bar_item_init(bar_item);
    bar_manager->bar_items[bar_manager->bar_item_count - 1] = bar_item;
    return bar_item;
}

void bar_manager_init(struct bar_manager *bar_manager)
{
  bar_manager->bars = NULL;
  bar_manager->bar_count = 0;
  bar_manager->bar_item_count = 0;
  bar_manager->display = "all";
  bar_manager->position = "top";
  bar_manager->height = 26;
  bar_manager->title = true;
  bar_manager->spaces = true;
  bar_manager->padding_left = 20;
  bar_manager->padding_right = 20;
  bar_manager->background_color = rgba_color_from_hex(0xff202020);
  bar_manager->foreground_color = rgba_color_from_hex(0xffa8a8a8);
  bar_manager->space_icon_color = rgba_color_from_hex(0xffd75f5f);
}

void bar_manager_update_components(struct bar_manager* bar_manager, uint32_t did, uint32_t sid) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    struct bar_item* bar_item = bar_manager->bar_items[i];
    bar_item_update_component(bar_item, did, sid);
  }
}

void bar_manager_script_update(struct bar_manager* bar_manager) {
  for (int i = 0; i < bar_manager->bar_item_count; i++) {
    bar_item_script_update(bar_manager->bar_items[i]);
  }
}

void bar_manager_begin(struct bar_manager *bar_manager)
{
  char * main = "main";
  char * all = "all";

  if (strcmp(bar_manager->display,main) == 0) {
    uint32_t did = display_manager_main_display_id();
    bar_manager->bars = (struct bar **) realloc(bar_manager->bars, sizeof(struct bar *) * 1);
    bar_manager->bar_count = 1;
    bar_manager->bars[0] = bar_create(did);
  } else if (strcmp(bar_manager->display,all) == 0) {
    bar_manager->bar_count = display_manager_active_display_count();
    bar_manager->bars = (struct bar **) realloc(bar_manager->bars, sizeof(struct bar *) * bar_manager->bar_count);

    for (uint32_t index=1; index <= bar_manager->bar_count; index++) {
      uint32_t did = display_manager_arrangement_display_id(index);
      bar_manager->bars[index - 1] = bar_create(did);
    }
  }
}
