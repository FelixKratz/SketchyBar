#include "bar_manager.h"

void bar_manager_set_foreground_color(struct bar_manager *bar_manager, uint32_t color)
{
    bar_manager->foreground_color = rgba_color_from_hex(color);
    if (bar_manager->_space_icon_strip) bar_manager_set_space_strip(bar_manager, bar_manager->_space_icon_strip);
    if (bar_manager->_power_icon_strip) bar_manager_set_power_strip(bar_manager, bar_manager->_power_icon_strip);
    if (bar_manager->_clock_icon) bar_manager_set_clock_icon(bar_manager, bar_manager->_clock_icon);
    if (bar_manager->_dnd_icon) bar_manager_set_dnd_icon(bar_manager, bar_manager->_dnd_icon);
    if (bar_manager->_space_icon) bar_manager_set_space_icon(bar_manager, bar_manager->_space_icon);
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

void bar_manager_set_battery_icon_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->battery_icon_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_power_icon_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->power_icon_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_clock_icon_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->clock_icon_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_dnd_icon_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->dnd_icon_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_text_font(struct bar_manager *bar_manager, char *font_string)
{
    if (bar_manager->t_font) {
        CFRelease(bar_manager->t_font);
    }

    if (font_string != bar_manager->t_font_prop) {
      if (bar_manager->t_font_prop) {
	free(bar_manager->t_font_prop);
      }

      bar_manager->t_font_prop = font_string;
    }

    bar_manager->t_font = bar_create_font(bar_manager->t_font_prop);
    bar_manager_refresh(bar_manager);
}

void bar_manager_set_icon_font(struct bar_manager *bar_manager, char *font_string)
{
  if (bar_manager->i_font) {
    CFRelease(bar_manager->i_font);
  }

  if (font_string != bar_manager->i_font_prop) {
    if (bar_manager->i_font_prop) {
      free(bar_manager->i_font_prop);
    }

    bar_manager->i_font_prop = font_string;
  }

  bar_manager->i_font = bar_create_font(bar_manager->i_font_prop);
  if (bar_manager->_space_icon_strip) bar_manager_set_space_strip(bar_manager, bar_manager->_space_icon_strip);
  if (bar_manager->_power_icon_strip) bar_manager_set_power_strip(bar_manager, bar_manager->_power_icon_strip);
    if (bar_manager->_clock_icon) bar_manager_set_clock_icon(bar_manager, bar_manager->_clock_icon);
    if (bar_manager->_space_icon) bar_manager_set_space_icon(bar_manager, bar_manager->_space_icon);
    if (bar_manager->_dnd_icon) bar_manager_set_dnd_icon(bar_manager, bar_manager->_dnd_icon);
    bar_manager_refresh(bar_manager);
}

void bar_manager_set_space_strip(struct bar_manager *bar_manager, char **icon_strip)
{
    for (int i = 0; i < buf_len(bar_manager->space_icon_strip); ++i) {
        bar_destroy_line(bar_manager->space_icon_strip[i]);
    }

    buf_free(bar_manager->space_icon_strip);
    bar_manager->space_icon_strip = NULL;

    if (icon_strip != bar_manager->_space_icon_strip) {
        for (int i = 0; i < buf_len(bar_manager->_space_icon_strip); ++i) {
            free(bar_manager->_space_icon_strip[i]);
        }

        buf_free(bar_manager->_space_icon_strip);
        bar_manager->_space_icon_strip = icon_strip;
    }

    for (int i = 0; i < buf_len(bar_manager->_space_icon_strip); ++i) {
        struct bar_line space_line = bar_prepare_line(bar_manager->i_font, bar_manager->_space_icon_strip[i], bar_manager->foreground_color);
        buf_push(bar_manager->space_icon_strip, space_line);
    }

    bar_manager_refresh(bar_manager);
}

void bar_manager_set_power_strip(struct bar_manager *bar_manager, char **icon_strip)
{
    if (bar_manager->battr_icon.line) {
        bar_destroy_line(bar_manager->battr_icon);
    }

    if (bar_manager->power_icon.line) {
        bar_destroy_line(bar_manager->power_icon);
    }

    if (icon_strip != bar_manager->_power_icon_strip) {
        for (int i = 0; i < buf_len(bar_manager->_power_icon_strip); ++i) {
            free(bar_manager->_power_icon_strip[i]);
        }

        buf_free(bar_manager->_power_icon_strip);
        bar_manager->_power_icon_strip = icon_strip;
    }

    if (buf_len(bar_manager->_power_icon_strip) == 2) {
      bar_manager->battr_icon = bar_prepare_line(bar_manager->i_font, bar_manager->_power_icon_strip[0], bar_manager->battery_icon_color);
      bar_manager->power_icon = bar_prepare_line(bar_manager->i_font, bar_manager->_power_icon_strip[1], bar_manager->power_icon_color);
    } else {
      bar_manager->battr_icon = bar_prepare_line(bar_manager->i_font, "", bar_manager->battery_icon_color);
      bar_manager->power_icon = bar_prepare_line(bar_manager->i_font, "", bar_manager->power_icon_color);
    }

bar_manager_refresh(bar_manager);
}

void bar_manager_set_clock_icon(struct bar_manager *bar_manager, char *icon)
{
    if (bar_manager->clock_icon.line) {
        bar_destroy_line(bar_manager->clock_icon);
    }

    if (icon != bar_manager->_clock_icon) {
        if (bar_manager->_clock_icon) {
            free(bar_manager->_clock_icon);
        }

        bar_manager->_clock_icon = icon;
    }

    bar_manager->clock_icon = bar_prepare_line(bar_manager->i_font, bar_manager->_clock_icon, bar_manager->foreground_color);

    bar_manager_refresh(bar_manager);
}

void bar_manager_set_clock_format(struct bar_manager *bar_manager, char *format)
{
    bar_manager->_clock_format = format;
    bar_manager_set_text_font(bar_manager, bar_manager->t_font_prop);
}


void bar_manager_set_space_icon(struct bar_manager *bar_manager, char *icon)
{
    if (bar_manager->space_icon.line) {
        bar_destroy_line(bar_manager->space_icon);
    }

    if (icon != bar_manager->_space_icon) {
        if (bar_manager->_space_icon) {
            free(bar_manager->_space_icon);
        }

        bar_manager->_space_icon = icon;
    }

    bar_manager->space_icon = bar_prepare_line(bar_manager->i_font, bar_manager->_space_icon, bar_manager->foreground_color);

    bar_manager_refresh(bar_manager);
}

void bar_manager_set_dnd_icon(struct bar_manager *bar_manager, char *icon)
{
  if (bar_manager->dnd_icon.line) {
    bar_destroy_line(bar_manager->dnd_icon);
  }

  if (icon != bar_manager->_dnd_icon) {
    if (bar_manager->_dnd_icon) {
      free(bar_manager->_dnd_icon);
    }

    bar_manager->_dnd_icon = icon;
  }

  bar_manager->dnd_icon = bar_prepare_line(bar_manager->i_font, bar_manager->_dnd_icon, bar_manager->dnd_icon_color);

  bar_manager_refresh(bar_manager);
}

void bar_manager_set_position(struct bar_manager *bar_manager, char *pos)
{
  bar_manager->position = pos;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_height(struct bar_manager *bar_manager, uint32_t height)
{
  bar_manager->height = height;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_spacing_left(struct bar_manager *bar_manager, uint32_t spacing)
{
  bar_manager->spacing_left = spacing;
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_spacing_right(struct bar_manager *bar_manager, uint32_t spacing)
{
  bar_manager->spacing_right = spacing;
  bar_manager_refresh(bar_manager);
}

void bar_manager_display_changed(struct bar_manager *bar_manager)
{
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

void bar_manager_init(struct bar_manager *bar_manager)
{
    bar_manager->bars = NULL;
    bar_manager->bar_count = 0;
    bar_manager_set_position(bar_manager, string_copy("top"));
    bar_manager_set_height(bar_manager, 26);
    bar_manager_set_spacing_left(bar_manager, 25);
    bar_manager_set_spacing_right(bar_manager, 15);
    bar_manager_set_text_font(bar_manager, string_copy("Helvetica Neue:Regular:10.0"));
    bar_manager_set_icon_font(bar_manager, string_copy("Font Awesome 5 Free:Regular:10.0"));
    bar_manager_set_background_color(bar_manager, 0xff202020);
    bar_manager_set_foreground_color(bar_manager, 0xffa8a8a8);
    bar_manager_set_space_icon_color(bar_manager, 0xffd75f5f);
    bar_manager_set_battery_icon_color(bar_manager, 0xffd75f5f);
    bar_manager_set_power_icon_color(bar_manager, 0xffcd950c);
    bar_manager_set_clock_icon_color(bar_manager, 0xffa8a8a8);
    bar_manager_set_clock_icon(bar_manager, string_copy(" "));
    bar_manager_set_clock_format(bar_manager, string_copy("%R"));
    bar_manager_set_space_icon(bar_manager, string_copy("*"));
    bar_manager_set_power_strip(bar_manager, NULL);
    bar_manager_set_dnd_icon(bar_manager, string_copy(""));
    bar_manager_set_dnd_icon_color(bar_manager, 0xffa8a8a8);
}

void bar_manager_begin(struct bar_manager *bar_manager)
{
    bar_manager->bar_count = display_manager_active_display_count();
    bar_manager->bars = (struct bar **) realloc(bar_manager->bars, sizeof(struct bar *) * bar_manager->bar_count);

    for (uint32_t index=1; index <= bar_manager->bar_count; index++)
    {
        uint32_t did = display_manager_arrangement_display_id(index);
        bar_manager->bars[index - 1] = bar_create(did);
    }
}
