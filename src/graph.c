#include "graph.h"

void graph_init(struct graph* graph) {
  graph->width = 0;
  graph->cursor = 0;

  graph->line_width = 0.5;
  graph->fill = true;
  graph->overrides_fill_color = false;
  graph->enabled = true;

  color_init(&graph->line_color, 0xffcccccc);
  color_init(&graph->fill_color, 0xffcccccc);
}

void graph_setup(struct graph* graph, uint32_t width) {
  graph->width = width;
  graph->y = malloc(sizeof(float) * width);
  memset(graph->y, 0, sizeof(float) * width);
}

float graph_get_y(struct graph* graph, uint32_t i) {
  if (!graph->enabled) return 0.f;
  return graph->y[ (graph->cursor + i)%graph->width ];
}

void graph_push_back(struct graph* graph, float y) {
  if (!graph->enabled) return;
  graph->y[graph->cursor] = y;

  ++graph->cursor;
  graph->cursor %= graph->width;
}

uint32_t graph_get_length(struct graph* graph) {
  if (graph->enabled) return graph->width;
  return 0;
}

void graph_calculate_bounds(struct graph* graph, uint32_t x, uint32_t y, uint32_t height) {
  graph->bounds.size.height = height;
  graph->bounds.origin.x = x;
  graph->bounds.origin.y = y - graph->bounds.size.height / 2
                           + graph->line_width;
}

void graph_draw(struct graph* graph, CGContextRef context) {
  uint32_t x =  graph->bounds.origin.x + (graph->rtl ? graph->width : 0);
  uint32_t y = graph->bounds.origin.y;
  uint32_t height = graph->bounds.size.height;

  uint32_t sample_width = 1;
  bool fill = graph->fill;
  CGContextSaveGState(context);
  CGContextSetRGBStrokeColor(context,
                             graph->line_color.r,
                             graph->line_color.g,
                             graph->line_color.b,
                             graph->line_color.a );

  if (graph->overrides_fill_color)
    CGContextSetRGBFillColor(context,
                             graph->fill_color.r,
                             graph->fill_color.g,
                             graph->fill_color.b,
                             graph->fill_color.a );
  else
    CGContextSetRGBFillColor(context,
                             graph->line_color.r,
                             graph->line_color.g,
                             graph->line_color.b,
                             0.2 * graph->line_color.a);

  CGContextSetLineWidth(context, graph->line_width);
  CGMutablePathRef p = CGPathCreateMutable();
  uint32_t start_x = x;
  if (graph->rtl) {
    CGPathMoveToPoint(p,
                      NULL,
                      x,
                      y + graph_get_y(graph, graph->width - 1) * height);

    for (int i = graph->width - 1; i > 0; --i, x -= sample_width) {
      CGPathAddLineToPoint(p, NULL, x, y + graph_get_y(graph, i) * height);
    }
  }
  else {
    CGPathMoveToPoint(p, NULL, x, y + graph_get_y(graph, 0) * height);
    for (int i = graph->width - 1; i > 0; --i, x += sample_width) {
      CGPathAddLineToPoint(p, NULL, x, y + graph_get_y(graph, i) * height);
    }
  }
  CGContextAddPath(context, p);
  CGContextStrokePath(context);
  if (fill) {
    if (graph->rtl) {
      CGPathAddLineToPoint(p, NULL, x + sample_width, y);
    }
    else {
      CGPathAddLineToPoint(p, NULL, x - sample_width, y);
    }
    CGPathAddLineToPoint(p, NULL, start_x, y);
    CGPathCloseSubpath(p);
    CGContextAddPath(context, p);
    CGContextFillPath(context);
  }
  CGPathRelease(p);
  CGContextRestoreGState(context);
}

void graph_serialize(struct graph* graph, char* indent, FILE* rsp) {
    fprintf(rsp, "%s\"color\": \"0x%x\",\n"
                 "%s\"fill_color\": \"0x%x\",\n"
                 "%s\"line_width\": \"%f\",\n"
                 "%s\"data\": [\n",
                 indent, graph->line_color.hex,
                 indent, graph->fill_color.hex,
                 indent, graph->line_width, indent);
    int counter = 0;
    for (int i = 0; i < graph->width; i++) {
      if (counter++ > 0) fprintf(rsp, ",\n");
      fprintf(rsp, "%s\t\"%f\"", indent, graph->y[i]);
    }
    fprintf(rsp, "\n%s]", indent);
}

void graph_destroy(struct graph* graph) {
  if (!graph->enabled) return;
  if (graph->y) free(graph->y);
  graph->y = NULL;
}

bool graph_parse_sub_domain(struct graph* graph, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_COLOR)) {
    return color_set_hex(&graph->line_color,
                         token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, PROPERTY_FILL_COLOR)) {
    graph->overrides_fill_color = true;
    return color_set_hex(&graph->fill_color,
                         token_to_uint32t(get_token(&message)));
  } else if (token_equals(property, PROPERTY_LINE_WIDTH)) {
    graph->line_width = token_to_float(get_token(&message));
    return true;
  } 
  else {
    struct key_value_pair key_value_pair = get_key_value_pair(property.text,
                                                              '.'           );
    if (key_value_pair.key && key_value_pair.value) {
      struct token subdom = {key_value_pair.key,strlen(key_value_pair.key)};
      struct token entry = {key_value_pair.value,strlen(key_value_pair.value)};
      if (token_equals(subdom, SUB_DOMAIN_COLOR)) {
        return color_parse_sub_domain(&graph->line_color, rsp, entry, message);
      }
      else if (token_equals(subdom, SUB_DOMAIN_FILL_COLOR)) {
        return color_parse_sub_domain(&graph->fill_color, rsp, entry, message);
      }
      else {
        respond(rsp, "[!] Graph: Invalid subdomain '%s'\n", subdom.text);
      }
    }
    else {
      respond(rsp, "[!] Graph: Invalid property '%s'\n", property.text);
    }
  }
  return false;
}
