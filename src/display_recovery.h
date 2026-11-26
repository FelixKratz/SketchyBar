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

// After wake, multi-display setups report valid but transient topologies for a
// moment. A rebuild waits until the same topology was seen for STABLE_NS, or
// until MAX_NS after wake; reuse of unchanged displays never waits.
#define DISPLAY_RECOVERY_RETRY_NS  250000000ULL
#define DISPLAY_RECOVERY_STABLE_NS 750000000ULL
#define DISPLAY_RECOVERY_MAX_NS   5000000000ULL

struct display_recovery_settle {
  uint64_t woke_at, changed_at;
  uint32_t count;
  struct display_recovery_slot last[32];
};

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

static inline bool display_recovery_same(
    const struct display_recovery_slot* a, uint32_t a_count,
    const struct display_recovery_slot* b, uint32_t b_count) {
  if (a_count != b_count) return false;
  for (uint32_t i = 0; i < a_count; ++i)
    if (a[i].did != b[i].did || a[i].adid != b[i].adid) return false;
  return true;
}

// Returns true when a rebuild may run now; otherwise the caller retries later.
static inline bool display_recovery_settled(
    struct display_recovery_settle* settle,
    const struct display_recovery_slot* current, uint32_t current_count,
    uint64_t now) {
  if (!settle->woke_at) return true;
  if (current_count > 32) current_count = 32;
  if (!display_recovery_same(settle->last, settle->count,
                             current, current_count)) {
    for (uint32_t i = 0; i < current_count; ++i) settle->last[i] = current[i];
    settle->count = current_count;
    settle->changed_at = now;
  }
  if (now - settle->woke_at >= DISPLAY_RECOVERY_MAX_NS
      || now - settle->changed_at >= DISPLAY_RECOVERY_STABLE_NS) {
    settle->woke_at = 0;
    return true;
  }
  return false;
}
