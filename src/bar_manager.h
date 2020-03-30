#ifndef BAR_MANAGER_H
#define BAR_MANAGER_H

struct bar_manager
{
    struct bar **bars;
    int bar_count;
    char *t_font_prop;
    char *i_font_prop;
    CTFontRef t_font;
    CTFontRef i_font;
    char **_space_icon_strip;
    char **_power_icon_strip;
    char *_clock_icon;
    char *_clock_format;
    char *_space_icon;
    struct rgba_color foreground_color;
    struct rgba_color background_color;
    struct rgba_color background_color_dim;
    struct bar_line *space_icon_strip;
    struct bar_line space_icon;
    struct bar_line clock_icon;
    struct bar_line battr_icon;
    struct bar_line power_icon;
    struct bar_line space_underline;
    struct bar_line power_underline;
    struct bar_line clock_underline;
};

void bar_manager_set_foreground_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_background_color(struct bar_manager *bar_manager, uint32_t color);
void bar_manager_set_text_font(struct bar_manager *bar_manager, char *font_string);
void bar_manager_set_icon_font(struct bar_manager *bar_manager, char *font_string);
void bar_manager_set_space_strip(struct bar_manager *bar_manager, char **icon_strip);
void bar_manager_set_power_strip(struct bar_manager *bar_manager, char **icon_strip);
void bar_manager_set_clock_icon(struct bar_manager *bar_manager, char *icon);
void bar_manager_set_clock_format(struct bar_manager *bar_manager, char *format);
void bar_manager_set_space_icon(struct bar_manager *bar_manager, char *icon);

void bar_manager_display_changed(struct bar_manager *bar_manager);
void bar_manager_refresh(struct bar_manager *bar_manager);
void bar_manager_resize(struct bar_manager *bar_manager);
void bar_manager_begin(struct bar_manager *bar_manager);
void bar_manager_init(struct bar_manager *bar_manager);


#endif
