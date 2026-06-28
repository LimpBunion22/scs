# Decision 0003: tactical UI evolution options

## Status

Accepted for the next prototype round. The final non-console UI stack remains
pending.

## Context

The console UI proved the data flow: UI emits commands, rendering consumes
presentation snapshots, and simulation advances through explicit ticks. It is
also too limited for sustained tactical play because static markers do not show
enough scale, motion, or engagement context.

The next work should improve tactical readability before adding a new UI stack.
That keeps the prototype focused on command decisions instead of windowing,
packaging, and visual polish.

## Options

| Option | Strength | Cost |
| --- | --- | --- |
| Improved console map | No dependency; fastest to test metrics and command flow. | Limited interaction and visual density. |
| Terminal TUI | Better panels, keyboard interaction, and logs while staying lightweight. | Adds dependency and terminal behavior differences. |
| Desktop UI with SFML/ImGui | Good fit for tactical map, overlays, and mouse input. | Requires dependency, rendering, input, and packaging decisions. |
| Web UI | Strong layout and iteration speed. | Adds runtime/tooling complexity and integration boundary work. |

## Decision

For the next round, keep the standard-library console UI but add tactical
reference metrics, missile/contact display, engagement commands, and a playable
engagement demo scenario. After that round, choose between:

- a terminal TUI if the main issue is panel layout and keyboard ergonomics;
- a desktop UI if map interaction, overlays, and mouse selection become the
  bottleneck.

Do not add a UI dependency until the owner approves the next UI stack decision.
The detailed selection process for the future UI can wait one more round.
Graphical interaction work should begin only after the UI stack is selected.

## Decision Criteria

- Can the player understand range, bearing, velocity, uncertainty, and weapon
  timing at a glance?
- Can commands be issued without direct ship control?
- Can automated time scaling remain visible and predictable?
- Can replay and simulation tests stay independent from UI rendering?
- Does the stack work on the expected development platforms without heavy setup?
