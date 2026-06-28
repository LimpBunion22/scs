# Decision 0002: minimal UI stack for UI-001

## Status

Accepted for the first tactical command map.

## Context

`UI-001` needs pan, zoom, selection, pause/resume, time-scale display, event log
display, and command emission against existing presentation snapshots. The
prototype still needs validation of command flow more than visual polish.

Adding a desktop UI dependency now would require choosing rendering, windowing,
input, packaging, and test strategy before the command-map contract has proven
itself.

## Decision

Use a standard-library console UI and ASCII tactical map for `UI-001`.

The app is split into:

- `src/ui/` for command parsing and UI-only state;
- `src/rendering/` for text rendering from tactical presentation snapshots;
- `src/app/` for wiring simulation, presentation, time-scale policy, UI, and
  rendering.

No third-party UI or rendering dependency is introduced by this decision.

## Consequences

The first map is interactively testable and deterministic, but not visually
polished. A future graphical UI can replace the console surface if it preserves
the same boundaries: UI emits commands, rendering consumes presentation data,
and simulation advances only by explicit ticks.
