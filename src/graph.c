#include "graph.h"

void graph_init(struct graph* graph, uint32_t width) {
  graph->width = width;
  graph->y = malloc(sizeof(float) * width);
  memset(graph->y, 0, sizeof(float) * width);
  graph->cursor = 0;

  graph->line_color = rgba_color_from_hex(0xcccccc);
  graph->fill_color = rgba_color_from_hex(0xcccccc);
  graph->line_width = 0.5; 
  graph->fill = true;
  graph->overrides_fill_color = false;
  graph->enabled = true;
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

void graph_calculate_bounds(struct graph* graph, uint32_t x, uint32_t y) {
  graph->bounds.origin.x = x;
  graph->bounds.origin.y = y - graph->bounds.size.height / 2 + graph->line_width;
}

void graph_draw(struct graph* graph, CGContextRef context) {
  uint32_t x =  graph->bounds.origin.x + (graph->rtl ? graph->width : 0);
  uint32_t y = graph->bounds.origin.y;
  uint32_t height = graph->bounds.size.height;

  uint32_t sample_width = 1;
  bool fill = graph->fill;
  CGContextSaveGState(context);
  CGContextSetRGBStrokeColor(context, graph->line_color.r, graph->line_color.g, graph->line_color.b, graph->line_color.a);
  if (graph->overrides_fill_color) CGContextSetRGBFillColor(context, graph->fill_color.r, graph->fill_color.g, graph->fill_color.b, graph->fill_color.a);
  else CGContextSetRGBFillColor(context, graph->line_color.r, graph->line_color.g, graph->line_color.b, 0.2 * graph->line_color.a);
  CGContextSetLineWidth(context, graph->line_width);
  CGMutablePathRef p = CGPathCreateMutable();
  uint32_t start_x = x;
  if (graph->rtl) {
    CGPathMoveToPoint(p, NULL, x, y + graph_get_y(graph, graph->width - 1) * height);
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

void graph_serialize(struct graph* graph, FILE* rsp) {
    fprintf(rsp, ",\n"
                 "\t\"graph\": {\n"
                 "\t\t\"graph.color\": \"0x%x\",\n"
                 "\t\t\"graph.fill_color\": \"0x%x\",\n"
                 "\t\t\"graph.line_width\": \"%f\",\n"
                 "\t\t\"data\": [\n",
                 hex_from_rgba_color(graph->line_color),
                 hex_from_rgba_color(graph->fill_color),
                 graph->line_width);
    int counter = 0;
    for (int i = 0; i < graph->width; i++) {
      if (counter++ > 0) fprintf(rsp, ",\n");
      fprintf(rsp, "\t\t\t\"%f\"",
              graph->y[i]);
    }
    fprintf(rsp, "\n\t]\n\t}");
}

void graph_destroy(struct graph* graph) {
  if (!graph->enabled) return;
  free(graph->y);
}

static bool graph_parse_sub_domain(struct graph* graph, FILE* rsp, struct token property, char* message) {
  if (token_equals(property, PROPERTY_COLOR)) {
    graph->line_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
    return true;
  } else if (token_equals(property, PROPERTY_FILL_COLOR)) {
    graph->fill_color = rgba_color_from_hex(token_to_uint32t(get_token(&message)));
    graph->overrides_fill_color = true;
    return true;
  } else if (token_equals(property, PROPERTY_LINE_WIDTH)) {
    graph->line_width = token_to_float(get_token(&message));
    return true;
  } 
  else {
    fprintf(rsp, "Unknown property: %s \n", property.text);
    printf("Unknown property: %s \n", property.text);
  }
  return false;
}
