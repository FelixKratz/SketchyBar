#include "context.h"
#include "animation.h"
#include "bar_manager.h"

CGContextRef context_create(CGSize size, CGFloat scale) {
  size_t pixel_width = (size_t)ceil(size.width * scale);
  size_t pixel_height = (size_t)ceil(size.height * scale);
  if (pixel_width == 0 || pixel_height == 0) return NULL;

  CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(NULL,
                                               pixel_width,
                                               pixel_height,
                                               8,
                                               pixel_width * 4,
                                               color_space,
                                               kCGImageAlphaPremultipliedFirst
                                               | kCGBitmapByteOrder32Host);
  CGColorSpaceRelease(color_space);
  if (!context) return NULL;

  CGContextScaleCTM(context, scale, scale);
  CGContextSetInterpolationQuality(context, kCGInterpolationNone);
  CGContextSetAllowsFontSmoothing(context, g_bar_manager.font_smoothing);
  return context;
}
