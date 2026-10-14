# Display recovery regression checks

Run the platform-independent decision tests from the repository root:

```sh
cc -std=c99 -Wall -Wextra -Werror -Isrc tests/display_recovery_test.c -o /tmp/sketchybar-display-recovery-test
/tmp/sketchybar-display-recovery-test
```

The same test can be compiled with `-fsanitize=address,undefined` where supported.
It covers unchanged single/dual displays, empty/unready enumeration, invalid IDs,
duplicate IDs, changed identities, ordering, disconnect/reconnect decisions, and
1,000 repeated unchanged snapshots. It does not exercise WindowServer or prove
that an active-display snapshot is complete during a physical transition.

## Manual acceptance checklist

Compare the base revision and this patch using the same configuration. Observe
the bar and process identity through password unlock and record whether existing
bar/item window identities survive an unchanged display arrangement. Do not
confuse a successful query with a visible frame or measured presentation latency.

- Repeated screen lock/unlock and genuine sleep/wake with one display.
- Built-in plus external display, including disconnect/reconnect while asleep.
- Display ordering, resolution/scaling, and main-display changes.
- `display=main`, `display=all`, and subsets whose selected monitor disappears.
- Fullscreen, sticky/non-sticky bars, hidden bars, popups, and animations.
- Screen Sharing reconnect and unlock on a desktop Mac.

## Evidence motivating the proposal

On an instrumented downstream build based on v2.24.0, two real MacBook unlocks
retained the same 134 window identities and 133 configured items with no recorded
topology rebuild. The user also reported that the bar stayed visually present.
Before the change, the process remained alive while window identities were
replaced repeatedly and queries sometimes timed out during recovery.

The downstream build also passed 20 simulated native sleep/display/wake cycles
on each of two Apple Silicon Macs. Those tests used a separate hidden one-item
server with diagnostic hooks, not real system sleep. The private configuration,
diagnostic hooks, packaging, and raw logs are not part of this contribution.
Those results are supporting evidence, not a claim that this cleaned-up branch
has passed the complete manual checklist above.

The standalone decision tests (including sanitizers) and an arm64 build pass for
this proposal. Wider physical display and Screen Sharing testing remains open.
