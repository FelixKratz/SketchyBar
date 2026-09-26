#include "symbol.h"
#include <AppKit/AppKit.h>

// The variable-value class method on NSImage. Used as the gating selector for
// the entire SF Symbols variable-rendering feature.
static SEL variable_value_selector(void) {
  return NSSelectorFromString(
    @"imageWithSystemSymbolName:variableValue:accessibilityDescription:"
  );
}

bool symbol_variable_rendering_available(void) {
  return [NSImage respondsToSelector:variable_value_selector()];
}

CGImageRef symbol_create_image(const char* name,
                               double value,
                               int mode,
                               double point_size,
                               uint32_t color,
                               bool has_color) {
  if (!name) return NULL;
  if (!symbol_variable_rendering_available()) return NULL;

  if (value < 0.0) value = 0.0;
  if (value > 1.0) value = 1.0;

  @autoreleasepool {
    NSString* ns_name = [NSString stringWithUTF8String:name];
    if (!ns_name) return NULL;

    NSImage* image = nil;
    if (@available(macOS 13.0, *)) {
      image = [NSImage imageWithSystemSymbolName:ns_name
                                   variableValue:value
                          accessibilityDescription:nil];
    }
    if (!image) return NULL;

    // NSImageSymbolConfiguration is macOS 11.0+. The variable-rendering call
    // above already requires macOS 13+, so we are always inside this branch
    // when we have an image, but the compiler needs the explicit availability
    // check for any reference to NSImageSymbolConfiguration.
    if (@available(macOS 11.0, *)) {
      NSImageSymbolConfiguration* config =
          [NSImageSymbolConfiguration configurationWithPointSize:point_size
                                                          weight:NSFontWeightRegular];


      if (has_color) {
        if (@available(macOS 12.0, *)) {
          CGFloat alpha = ((color >> 24) & 0xff) / 255.0;
          CGFloat red = ((color >> 16) & 0xff) / 255.0;
          CGFloat green = ((color >> 8) & 0xff) / 255.0;
          CGFloat blue = (color & 0xff) / 255.0;
          NSColor* ns_color = [NSColor colorWithCalibratedRed:red
                                                        green:green
                                                         blue:blue
                                                        alpha:alpha];
          NSImageSymbolConfiguration* color_config =
              [NSImageSymbolConfiguration configurationWithHierarchicalColor:ns_color];
          if (color_config) {
            config = config
              ? [config configurationByApplyingConfiguration:color_config]
              : color_config;
          }
        }
      }
      // Variable-value-mode configuration is macOS 26+. On older systems the
      // mode field is parsed at the SketchyBar layer for forward-compat but
      // not applied here, so Apple's default (automatic) rendering is used.
      if (mode != IMAGE_SYMBOL_MODE_AUTOMATIC) {
        if (@available(macOS 26.0, *)) {
          NSImageSymbolVariableValueMode ns_mode =
              NSImageSymbolVariableValueModeAutomatic;
          if (mode == IMAGE_SYMBOL_MODE_COLOR)
            ns_mode = NSImageSymbolVariableValueModeColor;
          else if (mode == IMAGE_SYMBOL_MODE_DRAW)
            ns_mode = NSImageSymbolVariableValueModeDraw;

          NSImageSymbolConfiguration* mode_config =
              [NSImageSymbolConfiguration
                  configurationWithVariableValueMode:ns_mode];
          if (mode_config) {
            config = config
              ? [config configurationByApplyingConfiguration:mode_config]
              : mode_config;
          }
        }
      }

      if (config) {
        NSImage* configured = [image imageWithSymbolConfiguration:config];
        if (configured) image = configured;
      }
    }

    NSRect proposed = NSMakeRect(0, 0, [image size].width, [image size].height);
    CGImageRef cg = [image CGImageForProposedRect:&proposed
                                          context:nil
                                            hints:nil];
    if (!cg) return NULL;
    CGImageRetain(cg);
    return cg;
  }
}
