#include "bar_manager.h"

void bar_manager_set_foreground_color(struct bar_manager *bar_manager, uint32_t color)
{
    bar_manager->foreground_color = rgba_color_from_hex(color);
    if (bar_manager->_space_icon_strip) bar_manager_set_space_strip(bar_manager, bar_manager->_space_icon_strip);
    if (bar_manager->_power_icon_strip) bar_manager_set_power_strip(bar_manager, bar_manager->_power_icon_strip);
    if (bar_manager->_clock_icon) bar_manager_set_clock_icon(bar_manager, bar_manager->_clock_icon);
    if (bar_manager->_dnd_icon) bar_manager_set_dnd_icon(bar_manager, bar_manager->_dnd_icon);
    if (bar_manager->_space_icon) bar_manager_set_space_icon(bar_manager, bar_manager->_space_icon);
    if (bar_manager->_left_shell_icon) bar_manager_set_left_shell_icon(bar_manager, bar_manager->_left_shell_icon);
    if (bar_manager->_right_shell_icon) bar_manager_set_right_shell_icon(bar_manager, bar_manager->_right_shell_icon);
    if (bar_manager->_display_separator_icon) bar_manager_set_display_separator_icon(bar_manager, bar_manager->_display_separator_icon);
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

void bar_manager_set_space_icon_color_secondary(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->space_icon_color_secondary = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_space_icon_color_tertiary(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->space_icon_color_tertiary = rgba_color_from_hex(color);
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

void bar_manager_set_left_shell_icon_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->left_shell_icon_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_right_shell_icon_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->right_shell_icon_color = rgba_color_from_hex(color);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_display_separator_icon_color(struct bar_manager *bar_manager, uint32_t color)
{
  bar_manager->display_separator_icon_color = rgba_color_from_hex(color);
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
    if (bar_manager->_left_shell_icon) bar_manager_set_left_shell_icon(bar_manager, bar_manager->_left_shell_icon);
    if (bar_manager->_right_shell_icon) bar_manager_set_right_shell_icon(bar_manager, bar_manager->_right_shell_icon);
    if (bar_manager->_display_separator_icon) bar_manager_set_display_separator_icon(bar_manager, bar_manager->_display_separator_icon);
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

void bar_manager_set_left_shell_icon(struct bar_manager *bar_manager, char *icon)
{
  if (bar_manager->left_shell_icon.line) {
    bar_destroy_line(bar_manager->left_shell_icon);
  }

  if (icon != bar_manager->_left_shell_icon) {
    if (bar_manager->_left_shell_icon) {
      free(bar_manager->_left_shell_icon);
    }

    bar_manager->_left_shell_icon = icon;
  }

  bar_manager->left_shell_icon = bar_prepare_line(bar_manager->i_font, bar_manager->_left_shell_icon, bar_manager->left_shell_icon_color);

  bar_manager_refresh(bar_manager);
}

void bar_manager_set_right_shell_icon(struct bar_manager *bar_manager, char *icon)
{
  if (bar_manager->right_shell_icon.line) {
    bar_destroy_line(bar_manager->right_shell_icon);
  }

  if (icon != bar_manager->_right_shell_icon) {
    if (bar_manager->_right_shell_icon) {
      free(bar_manager->_right_shell_icon);
    }

    bar_manager->_right_shell_icon = icon;
  }

  bar_manager->right_shell_icon = bar_prepare_line(bar_manager->i_font, bar_manager->_right_shell_icon, bar_manager->right_shell_icon_color);

  bar_manager_refresh(bar_manager);
}

void bar_manager_set_display_separator_icon(struct bar_manager *bar_manager, char *icon)
{
  if (bar_manager->display_separator_icon.line) {
    bar_destroy_line(bar_manager->display_separator_icon);
  }

  if (icon != bar_manager->_display_separator_icon) {
    if (bar_manager->_display_separator_icon) {
      free(bar_manager->_display_separator_icon);
    }

    bar_manager->_display_separator_icon = icon;
  }

  bar_manager->display_separator_icon = bar_prepare_line(bar_manager->i_font, bar_manager->_display_separator_icon, bar_manager->display_separator_icon_color);

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
  bar_manager_resize(bar_manager);
}

void bar_manager_set_spaces(struct bar_manager *bar_manager, bool value)
{
  bar_manager->spaces = value;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_spaces_for_all_displays(struct bar_manager *bar_manager, bool value)
{
  bar_manager->spaces_for_all_displays = value;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_display_separator(struct bar_manager *bar_manager, bool value)
{
  bar_manager->display_separator = value;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_clock(struct bar_manager *bar_manager, bool value)
{
  bar_manager->clock = value;
  bar_manager_resize(bar_manager);
}

void bar_manager_set_power(struct bar_manager *bar_manager, bool value)
{
  bar_manager->power = value;
  bar_manager_resize(bar_manager);
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

void bar_manager_set_left_shell(struct bar_manager *bar_manager, bool value)
{
  bar_manager->left_shell_on = value;
  set_shell_outputs(bar_manager);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_right_shell(struct bar_manager *bar_manager, bool value)
{
  bar_manager->right_shell_on = value;
  set_shell_outputs(bar_manager);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_center_shell(struct bar_manager *bar_manager, bool value)
{
  bar_manager->center_shell_on = value;
  set_shell_outputs(bar_manager);
  bar_manager_refresh(bar_manager);
}

void bar_manager_set_left_shell_output(struct bar_manager *bar_manager, char *output)
{
    bar_manager->left_shell_output = output;
}

void bar_manager_set_right_shell_output(struct bar_manager *bar_manager, char *output)
{
    bar_manager->right_shell_output = output;
}

void bar_manager_set_center_shell_output(struct bar_manager *bar_manager, char *output)
{
    bar_manager->center_shell_output = output;
}

void bar_manager_set_left_shell_command(struct bar_manager *bar_manager, char *command)
{
    bar_manager->left_shell_command = command;
    set_shell_outputs(bar_manager);
    bar_manager_refresh(bar_manager);
}

void bar_manager_set_right_shell_command(struct bar_manager *bar_manager, char *command)
{
    bar_manager->right_shell_command = command;
    set_shell_outputs(bar_manager);
    bar_manager_refresh(bar_manager);
}

void bar_manager_set_center_shell_command(struct bar_manager *bar_manager, char *command)
{
    bar_manager->center_shell_command = command;
    set_shell_outputs(bar_manager);
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


// TODO (cmacrae): Implement timeout
static char* run_shell(char *command)
{
    int cursor = 0;
    int bytes_read = 0;
    char *result = NULL;
    char buffer[BUFSIZ];

    FILE *handle = popen(command, "r");
    if (!handle) {
      return string_copy("error running command");
    }

    while ((bytes_read = read(fileno(handle), buffer, sizeof(buffer)-1)) > 0) {
        char *temp = realloc(result, cursor+bytes_read+1);
        if (!temp) goto err;

        result = temp;
        memcpy(result+cursor, buffer, bytes_read);
        cursor += bytes_read;
    }

    if (result && bytes_read != -1) {
        result[cursor] = '\0';
        return result;
    } else {
err:
	return string_copy("here we are");
        if (result) free(result);
    }

    pclose(handle);
    return result;
}

void set_shell_outputs(struct bar_manager *bar_manager)
{
  char* left_shell_output;
  char* right_shell_output;
  char* center_shell_output;
  if ((strlen(bar_manager->left_shell_command) > 0) && bar_manager->left_shell_on) {
    left_shell_output = run_shell(string_copy(bar_manager->left_shell_command));
    if (strlen(left_shell_output) > 0) {
      bar_manager_set_left_shell_output(bar_manager, left_shell_output);
    }
  }
  if ((strlen(bar_manager->right_shell_command) > 0) && bar_manager->right_shell_on) {
    right_shell_output = run_shell(string_copy(bar_manager->right_shell_command));
    if (strlen(right_shell_output) > 0) {
      bar_manager_set_right_shell_output(bar_manager, right_shell_output);
    }
  }
  if ((strlen(bar_manager->center_shell_command) > 0) && bar_manager->center_shell_on) {
    center_shell_output = run_shell(string_copy(bar_manager->center_shell_command));
    if (strlen(center_shell_output) > 0) {
      bar_manager_set_center_shell_output(bar_manager, center_shell_output);
    }
  }
}

void bar_manager_init(struct bar_manager *bar_manager)
{
    bar_manager->bars = NULL;
    bar_manager->bar_count = 0;
    bar_manager->display = "all";
    bar_manager->position = "top";
    bar_manager->height = 26;
    bar_manager->title = true;
    bar_manager->spaces = true;
    bar_manager->clock = true;
    bar_manager->power = true;
    bar_manager->padding_left = 20;
    bar_manager->padding_right = 20;
    bar_manager->spacing_left = 25;
    bar_manager->spacing_right = 15;
    bar_manager_set_text_font(bar_manager, string_copy("Helvetica Neue:Regular:12.0"));
    bar_manager_set_icon_font(bar_manager, string_copy("Font Awesome 5 Free:Solid:12.0"));
    bar_manager->background_color = rgba_color_from_hex(0xff202020);
    bar_manager->foreground_color = rgba_color_from_hex(0xffa8a8a8);
    bar_manager->space_icon_color = rgba_color_from_hex(0xffd75f5f);
    bar_manager->space_icon_color_secondary = rgba_color_from_hex(0xffd75f5f);
    bar_manager->space_icon_color_tertiary = rgba_color_from_hex(0xffd75f5f);
    bar_manager->battery_icon_color = rgba_color_from_hex(0xffd75f5f);
    bar_manager->power_icon_color = rgba_color_from_hex(0xffcd950c);
    bar_manager->clock_icon_color = rgba_color_from_hex(0xffa8a8a8);
    bar_manager_set_clock_icon(bar_manager, string_copy(""));
    bar_manager->_clock_format = "%R";
    bar_manager_set_space_icon(bar_manager, string_copy("•"));
    bar_manager_set_power_strip(bar_manager, NULL);
    bar_manager_set_dnd_icon(bar_manager, string_copy(""));
    bar_manager_set_dnd_icon_color(bar_manager, 0xffa8a8a8);
    bar_manager->left_shell_on = false;
    bar_manager->right_shell_on = false;
    bar_manager->center_shell_on = false;
    bar_manager_set_left_shell_icon(bar_manager, string_copy(""));
    bar_manager->left_shell_icon_color = rgba_color_from_hex(0xffa8a8a8);
    bar_manager_set_right_shell_icon(bar_manager, string_copy(""));
    bar_manager->right_shell_icon_color = rgba_color_from_hex(0xffa8a8a8);
    bar_manager->left_shell_output = "";
    bar_manager->right_shell_output = "";
    bar_manager->center_shell_output = "";
    bar_manager->center_shell_output = "";
    bar_manager->left_shell_command = "echo 'left shell'";
    bar_manager->right_shell_command = "echo 'right shell'";
    bar_manager->center_shell_command = "echo 'center shell'";
    bar_manager_set_display_separator_icon(bar_manager, string_copy("|"));
    bar_manager->display_separator_icon_color = rgba_color_from_hex(0xffa8a8a8);
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
