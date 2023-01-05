#include "slider.h"
#include "bar_manager.h"
#include "animation.h"

static bool slider_set_percentage(struct slider* slider, uint32_t percentage) {
  if (percentage == slider->percentage) return false;
  slider->percentage = min(percentage, 100);
  return true;
}

static bool slider_set_width(struct slider* slider, uint32_t width) {
  if (width == slider->background.bounds.size.width) return false;
  slider->background.bounds.size.width = width;
  return true;
}

static bool slider_set_foreground_color(struct slider* slider, uint32_t color) {
  if (slider->foreground_color == color) return false;
  slider->foreground_color = color;
  return background_set_color(&slider->foreground, color);
}

void slider_init(struct slider* slider) {
  slider->percentage = 0;
  slider->prev_drag_percentage = NO_DRAG;
  slider->background.bounds.size.width = 100;

  slider->foreground_color = 0xff0000ff;
  text_init(&slider->knob);
  background_init(&slider->background);
  background_init(&slider->foreground);
  background_set_color(&slider->background, 0xff000000);
  background_set_color(&slider->foreground, slider->foreground_color);
}

void slider_clear_pointers(struct slider* slider) {
  background_clear_pointers(&slider->background);
  background_clear_pointers(&slider->foreground);
  text_clear_pointers(&slider->knob);
}

void slider_setup(struct slider* slider, uint32_t width) {
  slider->background.bounds.size.width = width;
  background_set_enabled(&slider->background, true);
  background_set_enabled(&slider->foreground, true);
}

uint32_t slider_get_length(struct slider* slider) {
  return slider->background.bounds.size.width;
}

void slider_calculate_bounds(struct slider* slider, uint32_t x, uint32_t y) {
  background_calculate_bounds(&slider->background,
                              x,
                              y,
                              slider->background.bounds.size.width,
                              slider->background.bounds.size.height);

  background_calculate_bounds(&slider->foreground,
                              x,
                              y,
                              slider->background.bounds.size.width
                              * ((float)slider->percentage)/100.f,
                              slider->background.bounds.size.height);

  uint32_t knob_offset = max(min(((float)slider->percentage)/100.f
                                  * slider->background.bounds.size.width,
                                  slider->background.bounds.size.width
                                  - (slider->knob.bounds.size.width / 2.f + 1.f)
                             - slider->knob.bounds.size.width / 2.), 0.f);

  text_calculate_bounds(&slider->knob, x + knob_offset, y);
}

void slider_draw(struct slider* slider, CGContextRef context) {
  background_draw(&slider->background, context);
  background_draw(&slider->foreground, context);
  text_draw(&slider->knob, context);
}

void slider_destroy(struct slider* slider) {
  background_destroy(&slider->background);
  background_destroy(&slider->foreground);
  text_destroy(&slider->knob);
}

void slider_serialize(struct slider* slider, char* indent, FILE* rsp) {
  fprintf(rsp, "%s\"highlight_color\": \"0x%x\",\n"
               "%s\"percentage\": \"%d\",\n"
               "%s\"width\": \"%d\",\n",
               indent, slider->foreground_color,
               indent, slider->percentage,
               indent, (int)slider->background.bounds.size.width);

  char deeper_indent[strlen(indent) + 2];
  snprintf(deeper_indent, strlen(indent) + 2, "%s\t", indent);

  fprintf(rsp, "%s\"background\": {\n", indent);
  background_serialize(&slider->background, deeper_indent, rsp, false);
  fprintf(rsp, "\n%s},\n", indent);

  fprintf(rsp, "%s\"knob\": {\n", indent);
  text_serialize(&slider->knob, deeper_indent, rsp);
  fprintf(rsp, "\n%s}", indent);
}

bool slider_parse_sub_domain(struct slider* slider, FILE* rsp, struct token property, char* message) {
  bool needs_refresh = false;
  if (token_equals(property, PROPERTY_PERCENTAGE)) {
    struct token token = get_token(&message);
    ANIMATE(slider_set_percentage,
            slider,
            slider->percentage,
            token_to_uint32t(token));
  }
  else if (token_equals(property, PROPERTY_HIGHLIGHT_COLOR)) {
    struct token token = get_token(&message);
    ANIMATE_BYTES(slider_set_foreground_color,
                  slider,
                  slider->foreground_color,
                  token_to_uint32t(token)     );
  }
  else if (token_equals(property, PROPERTY_WIDTH)) {
    struct token token = get_token(&message);
    ANIMATE(slider_set_width,
            slider,
            slider->background.bounds.size.width,
            token_to_uint32t(token)              );
  }
  else if (token_equals(property, SUB_DOMAIN_KNOB)) {
    needs_refresh = text_set_string(&slider->knob,
                                    token_to_string(get_token(&message)),
                                    false                                );
  }
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text, '.');
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = { key_value_pair.key, strlen(key_value_pair.key) };
      struct token entry = { key_value_pair.value, strlen(key_value_pair.value) };
      if (token_equals(subdom, SUB_DOMAIN_BACKGROUND)) {
        background_parse_sub_domain(&slider->foreground, rsp, entry, message);
        background_set_color(&slider->foreground, slider->foreground_color);
        return background_parse_sub_domain(&slider->background, rsp, entry, message);
      }
      else if (token_equals(subdom, SUB_DOMAIN_KNOB))
        return text_parse_sub_domain(&slider->knob, rsp, entry, message);
      else {
        respond(rsp, "[!] Slider: Invalid subdomain '%s' \n", subdom.text);
      }
    }
    else {
      respond(rsp, "[!] Slider: Invalid property '%s'\n", property.text);
    }
  }

  return needs_refresh;
}
