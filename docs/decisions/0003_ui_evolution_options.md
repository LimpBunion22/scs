# Decision 0003: tactical UI evolution options

## Status

Accepted for implementation.

## Context

The console UI proved the data flow: UI emits commands, rendering consumes
presentation snapshots, and simulation advances through explicit ticks. The
enriched console pass now also exposes tactical reference metrics, missile
tracks, engagement command emission, and a playable engagement demo scenario.

That is enough to evaluate the next UI stack. The remaining bottleneck is not
data availability or command plumbing; it is spatial readability and direct map
interaction.

## Decision Criteria

- Can the player understand range, bearing, velocity, uncertainty, and weapon
  timing at a glance?
- Can commands be issued without direct ship control?
- Can automated time scaling remain visible and predictable?
- Can replay and simulation tests stay independent from UI rendering?
- Does the stack work on the expected development platforms without heavy setup?

## Evaluation

| Option | Fit against criteria | Cost and risk | Result |
| --- | --- | --- | --- |
| Improved console map | Preserves deterministic tests, existing command flow, event logs, and zero dependencies. Metrics are now visible but still text-heavy, and trajectories, uncertainty, and missile timing are not understandable at a glance. | Lowest cost, but another pass would mostly refine the current ceiling. | Keep as fallback and test surface, but do not use as the next primary UI. |
| Terminal TUI | Improves panels, logs, keyboard workflows, and dense tabular inspection. It still inherits terminal grid limits for map scale, overlays, cursor selection, and trajectory readability. | Adds a dependency plus terminal portability and input-mode differences without solving the main map-interaction problem. | Defer unless the owner rejects a graphical dependency. |
| Desktop SFML/ImGui | Best match for a 2D tactical command map with pan/zoom, overlays, uncertainty regions, missile tracks, mouse selection, inspectors, event log, and visible time controls. It can preserve the current C++ boundaries: UI emits commands, rendering consumes presentation data, and simulation ticks remain explicit. | Requires dependency, build, input, rendering, and packaging decisions. Tests should keep simulation/presentation independent and add focused UI smoke coverage. | Recommended next UI stack. |
| Web UI | Strong layout tools and fast iteration for panels and inspectors. A browser canvas can handle the map, but command/simulation integration would need a new process or serialization boundary. | Highest tooling and runtime complexity for the current C++ prototype; packaging and deterministic integration tests become broader. | Defer until distribution or remote UI needs justify the boundary work. |

## Decision

Recommend the next non-console UI be a desktop C++ UI using SFML for the 2D
window/input/rendering surface and ImGui for panels, inspectors, command
controls, event logs, and time controls.

The existing standard-library console UI must remain in place until the desktop
replacement is implemented and covered by tests. The desktop path is gated by
`SCS_BUILD_DESKTOP_UI` so default builds and core tests do not require graphical
dependencies. The new UI must keep the existing boundaries:

- UI emits `domain::Command` values and UI-only time-control state.
- Rendering consumes `presentation::TacticalSnapshot` and display state.
- Simulation advances only by explicit ticks outside the render frame loop.
- Replay and simulation tests remain independent from UI rendering.

This decision does not introduce the dependency by itself. The implementation
task must record the dependency acquisition, version expectation, build changes,
and smoke-test strategy before editing build tooling.

The initial desktop dependency pins are:

- installed SFML 2.6.x, with local development currently probing as 2.6.1;
- Dear ImGui v1.91.1;
- ImGui-SFML v2.6.1.

Do not upgrade to SFML 3 in this pass; ImGui-SFML 3.x requires SFML 3 and is a
separate migration decision.

## Initial Implementation Defaults

- Dependency bootstrap is an optional gated target using installed SFML and
  fetched or vendored Dear ImGui/ImGui-SFML.
- Selection uses nearest visible friendly/contact click priority with repeated
  click cycling for overlaps.
- Missiles are hover/inspector objects only in the first pass, not selectable
  command targets.
- Engagement uses staged selection: select a visible friendly launcher, select a
  visible hostile contact, then press an ImGui engagement button.
- Initial controls are pan, zoom, hover, selection, pause/resume, step/run,
  scale override, event log, command log, and engage-contact command emission.

## Consequences

The next UI work can validate the real tactical-map problem: spatial
readability, selection, overlays, and command flow under uncertainty. The cost
is accepting desktop UI dependency work, but that work is now tied to a proven
presentation and command contract instead of speculative rendering.
