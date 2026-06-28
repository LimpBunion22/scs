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
- The ImGui `Engage Contact` button emits `domain::EngageContactCommand` only
  when both staged objects are visible in the current tactical snapshot.
- The app submits emitted commands to simulation; UI code does not mutate
  simulation entities or missiles directly.

## Rendering

- SFML rendering consumes `presentation::TacticalSnapshot`,
  `rendering::TacticalMapProjection`, `rendering::TacticalSelection`, and hover
  highlight state.
- The renderer draws grid/reference space, friendly groups, hostile contacts,
  uncertainty circles, predicted trajectories, and missile tracks.
- Rendering must not query mutable simulation internals or recover hidden
  hostile entity truth.

## Time

- Tactical pause and manual scale remain UI/player state consumed by
  `gameplay::recommend_time_scale`.
- The desktop app advances simulation only from explicit `Step` or `Run` panel
  actions.
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
- Actual SFML/ImGui window behavior is smoke-tested manually unless a reliable
  headless render path is added.
