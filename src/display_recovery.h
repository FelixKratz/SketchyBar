#pragma once
#include <stdbool.h>
#include <stdint.h>

// Sleeping or partially enumerated displays must not discard retained windows.
enum display_recovery_action {
  DISPLAY_RECOVERY_DEFER,
  DISPLAY_RECOVERY_REUSE,
  DISPLAY_RECOVERY_REBUILD
};

struct display_recovery_slot { uint32_t did, adid; bool ready; };

static inline enum display_recovery_action display_recovery_decide(
    const struct display_recovery_slot* previous, uint32_t previous_count,
    const struct display_recovery_slot* current, uint32_t current_count) {
  if (!current_count || current_count > 32) return DISPLAY_RECOVERY_DEFER;
  for (uint32_t i = 0; i < current_count; ++i) {
    if (!current[i].ready || !current[i].did || !current[i].adid)
      return DISPLAY_RECOVERY_DEFER;
    for (uint32_t j = 0; j < i; ++j)
      if (current[j].did == current[i].did) return DISPLAY_RECOVERY_DEFER;
  }
  if (previous_count != current_count) return DISPLAY_RECOVERY_REBUILD;
  for (uint32_t i = 0; i < current_count; ++i)
    if (previous[i].did != current[i].did || previous[i].adid != current[i].adid)
      return DISPLAY_RECOVERY_REBUILD;
  return DISPLAY_RECOVERY_REUSE;
}
