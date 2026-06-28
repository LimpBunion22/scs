# Desktop UI Contract

The desktop UI is an optional SFML/Dear ImGui surface for the playable tactical
map. It must preserve the existing architecture: UI emits commands, rendering
consumes presentation snapshots, and simulation advances only by explicit ticks.

## Build

- Default builds do not require SFML, Dear ImGui, or ImGui-SFML.
- Enable the desktop target with:

```text
cmake -S . -B build-desktop -DSCS_BUILD_DESKTOP_UI=ON
cmake --build build-desktop --target scs_desktop
```

- The desktop target uses installed SFML 2.6.x and pins fetched or vendored
  Dear ImGui v1.91.1 and ImGui-SFML v2.6.1.
- SFML 3 and ImGui-SFML 3.x are out of scope for this pass.

## Interaction

- The map projection converts between screen pixels and world kilometers.
- Mouse wheel zoom keeps the world point under the cursor anchored.
- Right or middle mouse drag pans the map.
- Hover can identify visible friendly groups, visible hostile contacts, and
  player-visible missiles.
- Left click selects the nearest visible friendly group or hostile contact.
- Repeated clicks at the same screen point cycle overlapping selectable objects.
- Missiles are hover/inspector-only and do not become command selections.

## Command Flow

- Desktop selection stores a staged launcher and staged target separately from
  the active inspector selection.
- Selecting a friendly group stages the launcher.
- Selecting a hostile contact stages the target.
- The ImGui `Set Velocity` button emits `domain::SetVelocityCommand` only when
  the selected friendly or staged launcher is visible in the current tactical
  snapshot.
- The ImGui `Engage Contact` button emits `domain::EngageContactCommand` only
  when both staged objects are visible in the current tactical snapshot.
- The app submits emitted commands to simulation; UI code does not mutate
  simulation entities or missiles directly.

## Panels

- ImGui tactical panels consume a pure `ui::DesktopPanelModel` derived only
  from `presentation::TacticalSnapshot`, desktop UI state, and
  `gameplay::TimeScaleRecommendation`.
- Desktop maneuver controls store typed velocity components in desktop UI state
  and resolve a visible friendly target without mutating simulation state.
- The panel model exposes:
  - selected friendly position, velocity, speed, ammunition, and defense
    charges;
  - selected hostile contact observer, estimate, confidence, uncertainty, age,
    range, bearing, closing speed, and deterministic closest-approach metrics
    when the observer is visible;
  - player-visible missile ID, launcher, target contact when known, position,
    velocity, speed, and status;
  - staged launcher and staged target value plus readiness status.
- Observer-dependent contact metrics must be `unknown` when the observer is not
  visible in the tactical snapshot.
- Missile target display must be `unknown` when no player-visible target contact
  is available.
- ImGui renders one metric per row inside the fixed side panel so the default
  desktop layout stays readable without relying on the ASCII renderer.

## Rendering

- SFML rendering consumes `presentation::TacticalSnapshot`,
  a pure `rendering::TacticalMapOverlay` derived from
  `presentation::TacticalSnapshot` plus `rendering::TacticalMapProjection`, and
  `rendering::TacticalSelection` plus hover highlight state.
- The renderer draws grid/reference space, friendly groups, hostile contacts,
  uncertainty circles, predicted trajectories, and missile tracks.
- The overlay model computes visible world bounds, scale-aware major/minor grid
  spacing, scale-bar length, marker and uncertainty display radii, and
  off-screen edge hints without querying simulation state.
- Selected and hovered friendlies, contacts, and missiles remain visually
  distinct at both visible positions and off-screen edge hints.
- Rendering must not query mutable simulation internals or recover hidden
  hostile entity truth.

## Time

- Tactical pause and manual scale remain UI/player state consumed by
  `gameplay::recommend_time_scale`.
- The desktop app owns `ui::DesktopTimeController`, which exposes paused/running
  state, explicit step requests, elapsed-time accumulation, fractional carry, and
  a maximum per-frame catch-up clamp without SFML or ImGui dependencies.
- `Pause` disables continuous advancement and clears accumulated elapsed
  simulation time.
- `Run` enables continuous advancement. Each app frame passes explicit elapsed
  seconds, the current `gameplay::TimeScaleRecommendation`, and the simulation
  fixed-step size to the controller; only whole ticks returned by the controller
  may be submitted to simulation.
- `Step` requests exactly one fixed tick and works while paused.
- Render frame time must not advance simulation directly.

## Tests

- Default checks remain:

```text
cmake --build build
ctest --test-dir build --output-on-failure
```

- Non-graphical desktop interaction tests cover projection, zoom anchoring,
  hit testing, overlap cycling, missile hover-only behavior, and staged command
  emission.
- Non-graphical desktop command-flow regression covers selection, staged
  velocity and engage-contact command emission, explicit simulation ticks,
  missile launch/threat/resolution events, and replay equivalence.
- Actual SFML/ImGui window behavior is smoke-tested manually unless a reliable
  headless render path is added.
