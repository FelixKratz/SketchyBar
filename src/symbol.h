#pragma once
#include <CoreGraphics/CoreGraphics.h>
#include <stdint.h>
#include <stdbool.h>

#define IMAGE_SYMBOL_MODE_AUTOMATIC 0
#define IMAGE_SYMBOL_MODE_COLOR     1
#define IMAGE_SYMBOL_MODE_DRAW      2

// Returns true if variable-value rendering of SF Symbols is available at
// runtime on this macOS version. Selector check; safe on macOS 10.13+ even
// though the call sites only trigger on macOS 13+ in practice.
bool symbol_variable_rendering_available(void);

// Renders an SF Symbol image with the requested variable value. Returns a
// retained CGImageRef the caller must CGImageRelease, or NULL if the symbol
// name is unknown, variable rendering is unavailable, or rendering failed.
//
//   name       Symbol name (e.g. "battery.100percent.circle").
//   value      Variable value in [0, 1]; clamped here as well.
//   mode       One of IMAGE_SYMBOL_MODE_*; applied only on macOS 26+, ignored
//              on older systems (Apple default behavior is used instead).
//   point_size Symbol configuration point size, in points (e.g. 32.0).
//   color      Optional ARGB tint color; applied when has_color is true.
CGImageRef symbol_create_image(const char* name,
                               double value,
                               int mode,
                               double point_size,
                               uint32_t color,
                               bool has_color);
