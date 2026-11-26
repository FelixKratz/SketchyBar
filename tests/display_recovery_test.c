#include <assert.h>
#include <stdio.h>
#include "display_recovery.h"

int main(void) {
  struct display_recovery_slot single[] = {{1, 1, true}};
  struct display_recovery_slot other[] = {{4, 1, true}};
  struct display_recovery_slot dual[] = {{4, 1, true}, {1, 2, true}};
  struct display_recovery_slot reversed[] = {{1, 1, true}, {4, 2, true}};
  struct display_recovery_slot incomplete[] = {{4, 1, true}, {1, 0, false}};
  struct display_recovery_slot duplicate[] = {{1, 1, true}, {1, 2, true}};
  struct display_recovery_slot invalid_id[] = {{0, 1, true}};
  struct display_recovery_slot invalid_arrangement[] = {{1, 0, true}};
  struct display_recovery_slot unready[] = {{1, 1, false}};

  assert(display_recovery_decide(single, 1, single, 1) == DISPLAY_RECOVERY_REUSE);
  assert(display_recovery_decide(other, 1, other, 1) == DISPLAY_RECOVERY_REUSE);
  assert(display_recovery_decide(dual, 2, dual, 2) == DISPLAY_RECOVERY_REUSE);
  assert(display_recovery_decide(dual, 2, NULL, 0) == DISPLAY_RECOVERY_DEFER);
  assert(display_recovery_decide(dual, 2, NULL, 33) == DISPLAY_RECOVERY_DEFER);
  assert(display_recovery_decide(dual, 2, incomplete, 2) == DISPLAY_RECOVERY_DEFER);
  assert(display_recovery_decide(dual, 2, duplicate, 2) == DISPLAY_RECOVERY_DEFER);
  assert(display_recovery_decide(single, 1, invalid_id, 1) == DISPLAY_RECOVERY_DEFER);
  assert(display_recovery_decide(single, 1, invalid_arrangement, 1) == DISPLAY_RECOVERY_DEFER);
  assert(display_recovery_decide(single, 1, unready, 1) == DISPLAY_RECOVERY_DEFER);
  assert(display_recovery_decide(dual, 2, single, 1) == DISPLAY_RECOVERY_REBUILD);
  assert(display_recovery_decide(single, 1, dual, 2) == DISPLAY_RECOVERY_REBUILD);
  assert(display_recovery_decide(dual, 2, reversed, 2) == DISPLAY_RECOVERY_REBUILD);
  assert(display_recovery_decide(single, 1, other, 1) == DISPLAY_RECOVERY_REBUILD);
  assert(display_recovery_decide(NULL, 0, single, 1) == DISPLAY_RECOVERY_REBUILD);
  for (int i = 0; i < 1000; ++i)
    assert(display_recovery_decide(dual, 2, dual, 2) == DISPLAY_RECOVERY_REUSE);

  // Wake settling: a transient but valid topology waits, a stable one rebuilds.
  const uint64_t ms = 1000000ULL;
  struct display_recovery_settle settle = {0};
  assert(display_recovery_settled(&settle, single, 1, 10 * ms));
  settle = (struct display_recovery_settle){ .woke_at = 1000 * ms,
                                             .changed_at = 1000 * ms };
  assert(!display_recovery_settled(&settle, single, 1, 1100 * ms));
  assert(!display_recovery_settled(&settle, reversed, 2, 1350 * ms));
  assert(!display_recovery_settled(&settle, reversed, 2, 1850 * ms));
  assert(display_recovery_settled(&settle, reversed, 2, 2100 * ms));
  assert(settle.woke_at == 0);
  settle = (struct display_recovery_settle){ .woke_at = 1000 * ms,
                                             .changed_at = 1000 * ms };
  for (uint64_t t = 1100; t < 6000; t += 500)
    assert(!display_recovery_settled(&settle, (t / 500) % 2 ? single : dual,
                                     (t / 500) % 2 ? 1 : 2, t * ms));
  assert(display_recovery_settled(&settle, single, 1, 6000 * ms));

  puts("Display recovery decision tests passed.");
  return 0;
}
