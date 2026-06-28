# Agent task contracts

These contracts are the current integration queue for disposable agents. Each agent should read `AGENTS.md`, this file, and only the relevant source and tests named in its task.

## SCENARIO-001 — Core scenario regression

Task:
SCENARIO-001 — Core scenario regression

Objective:
Make the two-group vertical-slice scenario explicit, stable, and regression-tested as the baseline for later sensors, contacts, missiles, and UI.

Relevant files:
- `src/simulation/scenario.*`
- `src/simulation/replay.*`
- `tests/simulation/deterministic_replay_tests.cpp`
- `docs/contracts/simulation_core.md`

Allowed files:
- `src/simulation/scenario.*`
- `tests/simulation/*`
- `docs/mechanics/scenario.md`
- `CMakeLists.txt`, only to add a new test target if needed

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/decisions/0001_vertical_slice_sequence.md`

Invariants:
- No rendering or UI dependency.
- No third-party dependency.
- Scenario data remains deterministic.
- Existing replay tests continue to pass.

Acceptance criteria:
- Scenario includes exactly two combat groups with stable IDs, names, allegiance, positions, velocities, and sensor ranges.
- Test verifies initial state and fixed-tick positions after a known duration.
- Test verifies replay produces identical events and snapshots.
- Short mechanics doc records units and scenario intent.

Deliverables:
- Implementation.
- Tests.
- `docs/mechanics/scenario.md`.
- Completion report.

Out of scope:
- JSON scenario loading.
- Procedural generation.
- UI rendering.
- Sensors, contacts, missiles, or AI.

## CONTACT-001 — Simplified sensors and uncertain contacts

Task:
CONTACT-001 — Simplified sensors and uncertain contacts

Objective:
Implement deterministic contact creation/update for entities inside sensor range, with uncertainty growth while a contact is unobserved.

Relevant files:
- `src/domain/entity.h`
- `src/domain/event.h`
- `src/domain/snapshot.h`
- `src/simulation/simulation.*`
- `tests/simulation/*`

Allowed files:
- `src/domain/contact.h`
- `src/domain/event.h`
- `src/domain/snapshot.h`
- `src/simulation/contact_tracker.*`
- `src/simulation/simulation.*`
- `tests/simulation/contact_tests.cpp`
- `docs/contracts/contact_snapshot.md`
- `docs/mechanics/sensors.md`
- `CMakeLists.txt`

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/decisions/0001_vertical_slice_sequence.md`

Invariants:
- Contacts are estimates, not direct enemy entity handles.
- Contact update order is stable by observer ID, then target ID.
- Uncertainty grows monotonically when unobserved.
- Fresh observation reduces or resets uncertainty deterministically.
- Snapshot data is read-only.
- No UI, rendering, random noise, or probabilistic filter.

Acceptance criteria:
- Friendly entities can create hostile contact tracks when hostile entities enter sensor range.
- Contact snapshot contains contact ID, observer ID, estimated position, estimated velocity, last observed tick, confidence, classification, and uncertainty radius.
- Contact remains after leaving sensor range and its uncertainty grows each tick.
- Tests cover creation, refresh, stale growth, and replay determinism.
- Events are emitted for first detection and contact update with explicit severity.

Deliverables:
- Implementation.
- Tests.
- `docs/contracts/contact_snapshot.md`.
- `docs/mechanics/sensors.md`.
- Completion report.

Out of scope:
- Multi-sensor fusion.
- Kalman filters.
- Sensor occlusion.
- Rendering uncertainty regions.
- Detection probability.

## PRESENT-001 — Tactical presentation snapshot

Task:
PRESENT-001 — Tactical presentation snapshot

Objective:
Create a read-only presentation layer that converts simulation snapshots and events into tactical-map data without mutating simulation state.

Relevant files:
- `src/domain/snapshot.h`
- `src/domain/event.h`
- `src/simulation/simulation.*`
- `docs/contracts/simulation_core.md`

Allowed files:
- `src/presentation/tactical_snapshot.*`
- `src/presentation/prediction.*`
- `tests/presentation/*`
- `docs/contracts/tactical_snapshot.md`
- `CMakeLists.txt`

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/contracts/contact_snapshot.md`, if it exists
- `docs/decisions/0001_vertical_slice_sequence.md`

Invariants:
- Presentation must not mutate simulation.
- Presentation must not depend on UI or rendering libraries.
- Prediction must be deterministic and derived only from snapshot data.
- Keep map units in kilometers.

Acceptance criteria:
- Tactical snapshot separates friendly entities, hostile contacts, events, and predicted trajectories.
- Prediction covers inertial movement for a configurable number of ticks.
- Tests verify no simulation state changes when generating presentation data.
- Contract doc records fields intended for the first tactical map.

Deliverables:
- Implementation.
- Tests.
- `docs/contracts/tactical_snapshot.md`.
- Completion report.

Out of scope:
- UI controls.
- Drawing code.
- Camera.
- Missile visualization unless `MISSILE-001` has already landed.

## TIME-001 — Adaptive time-scale recommendations

Task:
TIME-001 — Adaptive time-scale recommendations

Objective:
Implement a deterministic policy that recommends simulation time scale from recent event severity while remaining separate from simulation state advancement.

Relevant files:
- `src/domain/event.h`
- `src/simulation/simulation.*`
- `tests/simulation/deterministic_replay_tests.cpp`

Allowed files:
- `src/gameplay/time_scale_policy.*`
- `tests/gameplay/time_scale_policy_tests.cpp`
- `docs/contracts/time_scale_policy.md`
- `docs/mechanics/time_scale.md`
- `CMakeLists.txt`

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/decisions/0001_vertical_slice_sequence.md`

Invariants:
- Time-scale policy does not advance simulation.
- Tactical pause is represented as a player/app state, not hidden simulation behavior.
- Policy input is explicit event data and current tick.
- Same event stream and tick produce the same recommendation.
- No wall-clock dependency.

Acceptance criteria:
- Policy maps `Info`, `Advisory`, `Threat`, and `Critical` severities to documented recommended scales.
- Policy exposes a reason string or enum suitable for UI display.
- Tests cover quiet time, advisory/contact activity, threat, critical, and player override behavior.
- Existing simulation tests continue to pass.

Deliverables:
- Implementation.
- Tests.
- `docs/contracts/time_scale_policy.md`.
- `docs/mechanics/time_scale.md`.
- Completion report.

Out of scope:
- UI controls.
- Real-time scheduling.
- Frame pacing.
- Missile-specific time slowdown unless missile events already exist.

## MISSILE-001 — One missile and one defensive response

Task:
MISSILE-001 — One missile and one defensive response

Objective:
Implement a deterministic missile engagement from command to resolution, using one missile type and one defensive response.

Relevant files:
- `src/domain/command.h`
- `src/domain/entity.h`
- `src/domain/event.h`
- `src/domain/snapshot.h`
- `src/simulation/simulation.*`
- `tests/simulation/*`

Allowed files:
- `src/domain/command.h`
- `src/domain/entity.h`
- `src/domain/event.h`
- `src/domain/snapshot.h`
- `src/simulation/missile.*`
- `src/simulation/simulation.*`
- `tests/simulation/missile_tests.cpp`
- `docs/contracts/engagement_commands.md`
- `docs/mechanics/missiles.md`
- `CMakeLists.txt`

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/contracts/contact_snapshot.md`
- `docs/decisions/0001_vertical_slice_sequence.md`

Invariants:
- Engagement command targets a contact or entity ID explicitly.
- Missile movement is deterministic fixed-step movement.
- One defensive response is deterministic and explicitly evented.
- Ammunition state is explicit and replayable.
- No damage model beyond alive/destroyed or hit/miss result.

Acceptance criteria:
- Command launches one missile when target and ammunition are valid.
- Missile track appears in snapshots.
- Defensive response can change the deterministic outcome.
- Events explain launch, approach/threat, defensive response, and hit/miss.
- Replay test verifies identical final snapshot and event stream.

Deliverables:
- Implementation.
- Tests.
- `docs/contracts/engagement_commands.md`.
- `docs/mechanics/missiles.md`.
- Completion report.

Out of scope:
- Multiple weapon families.
- Detailed projectile physics.
- Component damage.
- Salvos.
- UI order creation.

## UI-001 — Minimal tactical command map

Task:
UI-001 — Minimal tactical command map

Objective:
Build the first interactive tactical command map around presentation snapshots, with pan/zoom, selection, pause, time-scale display, event log, and command emission.

Relevant files:
- `src/app/main.cpp`
- `src/presentation/*`
- `src/domain/command.h`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/time_scale_policy.md`

Allowed files:
- `src/app/*`
- `src/ui/*`
- `src/rendering/*`
- `tests/integration/*`
- `docs/contracts/ui_commands.md`
- `docs/decisions/0002_ui_stack.md`
- `CMakeLists.txt`

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/time_scale_policy.md`
- `docs/decisions/0001_vertical_slice_sequence.md`

Invariants:
- UI emits commands; it does not directly mutate simulation entities.
- Rendering consumes presentation snapshots only.
- Simulation advances by explicit ticks independent of render frame rate.
- New third-party dependency requires an architectural decision record.

Acceptance criteria:
- User can pan and zoom the map.
- User can inspect friendly group and contacts.
- User can pause/resume.
- User can see current simulation time scale and reason.
- User can see a command/event log.
- User can issue at least one existing command through the UI.

Deliverables:
- Implementation.
- Integration smoke test or manual test notes.
- `docs/decisions/0002_ui_stack.md`.
- `docs/contracts/ui_commands.md`.
- Completion report.

Out of scope:
- Campaign UI.
- Ship construction.
- 3D rendering.
- Multiplayer.
- Visual polish beyond readability.

## REPLAY-001 — Vertical slice replay regression

Task:
REPLAY-001 — Vertical slice replay regression

Objective:
Add a full vertical-slice replay regression that verifies deterministic behavior across scenario setup, contacts, time-relevant events, missile engagement, and final state.

Relevant files:
- `src/domain/*`
- `src/simulation/*`
- `src/gameplay/*`
- `tests/*`
- `docs/contracts/*`

Allowed files:
- `tests/integration/vertical_slice_replay_tests.cpp`
- `src/simulation/replay.*`
- `docs/contracts/replay.md`
- `CMakeLists.txt`

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/contracts/contact_snapshot.md`
- `docs/contracts/engagement_commands.md`
- `docs/contracts/time_scale_policy.md`
- `docs/decisions/0001_vertical_slice_sequence.md`

Invariants:
- Test uses explicit scenario, command stream, seed, and tick count.
- Test does not depend on UI, wall-clock time, random device, or filesystem order.
- Failure output should identify first divergent snapshot or event.

Acceptance criteria:
- Integration test runs the same replay twice and compares final snapshot and event stream.
- Test covers at least one contact detection and one missile engagement.
- Contract doc records replay inputs and comparison rules.
- Existing tests continue to pass.

Deliverables:
- Integration test.
- Replay contract doc.
- Completion report.

Out of scope:
- Save-file format.
- Binary replay serialization.
- Cross-platform floating-point guarantees.

---

## Next round — playable tactical map

The first round produced a deterministic core and a minimal console map. The
next round should make the prototype readable and playable before choosing a
new UI stack.

Recommended sequence:

1. `PRESENT-002` — player tactical view and missile tracks.
2. `DISPLAY-001` — tactical metrics and map references.
3. `UI-002` — engagement command emission.
4. `SCENARIO-002` — playable engagement demo scenario.
5. `UIEVAL-001` — UI stack decision after the enriched console pass.

Owner decisions recorded:

- `scs_demo` should start in the new playable engagement scenario once
  `SCENARIO-002` lands.
- The new UI selection process can wait one more round.
- Graphical interaction work should start only after the next UI stack is
  selected.

Owner decisions still pending:

- whether the next non-console UI should be terminal TUI, desktop SFML/ImGui,
  or web;
- after UI stack selection, what object-selection model the graphical UI should
  use.

## PRESENT-002 — Player tactical view and missile tracks

Task:
PRESENT-002 — Player tactical view and missile tracks

Objective:
Make tactical presentation explicitly player-facing by filtering contacts to
friendly-owned tracks and exposing missile tracks without leaking hidden hostile
entity state.

Relevant files:
- `src/domain/snapshot.h`
- `src/presentation/tactical_snapshot.*`
- `tests/presentation/tactical_snapshot_tests.cpp`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/tactical_display_metrics.md`

Allowed files:
- `src/presentation/tactical_snapshot.*`
- `tests/presentation/*`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/tactical_display_metrics.md`
- `CMakeLists.txt`, only if a new presentation test target is needed

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/contracts/contact_snapshot.md`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/tactical_display_metrics.md`

Invariants:
- Presentation must not mutate simulation.
- Presentation must not depend on UI or rendering.
- Tactical snapshots must remain read-only copies or pure derived data.
- Hostile entity truth must not be exposed through player-facing presentation.
- No simulation behavior changes.

Acceptance criteria:
- `hostile_contacts` contains only contacts whose observer is a visible friendly entity.
- Tactical presentation exposes missile tracks needed by the display contract.
- Friendly-launched missiles can show launcher ID, target contact ID when known, position, velocity, and status.
- Presentation does not expose hidden hostile entity snapshots to satisfy missile display.
- Tests cover contact filtering, missile track exposure, and no mutation of simulation state.
- `docs/contracts/tactical_snapshot.md` is updated to describe the new fields and visibility rule.

Deliverables:
- Implementation.
- Tests.
- Updated tactical snapshot contract.
- Completion report.

Out of scope:
- New missile mechanics.
- Rendering changes.
- UI command changes.
- Damage model.

## DISPLAY-001 — Tactical metrics and map references

Task:
DISPLAY-001 — Tactical metrics and map references

Objective:
Upgrade the tactical map display from static markers to an information display
with reference frame, scale, selected-object metrics, contact quality, and
missile status.

Relevant files:
- `src/rendering/tactical_map_renderer.*`
- `src/presentation/tactical_snapshot.*`
- `src/gameplay/time_scale_policy.*`
- `tests/integration/ui_command_map_tests.cpp`
- `docs/contracts/tactical_display_metrics.md`

Allowed files:
- `src/rendering/*`
- `tests/integration/*`
- `docs/contracts/tactical_display_metrics.md`
- `CMakeLists.txt`, only if a new rendering test target is needed

Required context:
- `AGENTS.md`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/tactical_display_metrics.md`
- `docs/contracts/time_scale_policy.md`

Invariants:
- Rendering consumes presentation snapshots and UI display state only.
- Rendering must not query simulation internals.
- Rendering must not emit commands or advance simulation.
- Metrics must be deterministic pure derivations from display inputs.
- Missing reference data must be shown as unknown or omitted, not guessed.

Acceptance criteria:
- Map header shows tick, elapsed time, map center, kilometers per cell, visible span, and orientation.
- Friendly selection shows position, velocity, speed, ammunition, and defensive charges.
- Contact selection shows estimated position, estimated velocity, confidence, uncertainty, age, range, bearing, closing speed, and closest approach relative to its observer when available.
- Missile tracks are listed when present, with ID, launcher, target contact when available, position, velocity, speed, and status.
- Event log still shows severity, tick, type, subject, and message.
- Tests assert representative metric labels and values in rendered output.

Deliverables:
- Implementation.
- Tests.
- Completion report.

Out of scope:
- Graphical UI dependency.
- Mouse input.
- New presentation fields except those delivered by `PRESENT-002`.
- New simulation mechanics.

## UI-002 — Engagement command emission

Task:
UI-002 — Engagement command emission

Objective:
Allow the player to issue the existing contact engagement command through the
tactical UI without direct ship control.

Relevant files:
- `src/ui/tactical_command_ui.*`
- `src/domain/command.h`
- `tests/integration/ui_command_map_tests.cpp`
- `docs/contracts/ui_commands.md`
- `docs/contracts/engagement_commands.md`

Allowed files:
- `src/ui/*`
- `tests/integration/*`
- `docs/contracts/ui_commands.md`
- `CMakeLists.txt`, only if a new UI test target is needed

Required context:
- `AGENTS.md`
- `docs/contracts/ui_commands.md`
- `docs/contracts/engagement_commands.md`
- `docs/contracts/tactical_snapshot.md`

Invariants:
- UI emits command data only.
- UI does not mutate simulation entities or missiles.
- Simulation remains responsible for validating launcher, contact ownership, target state, and ammunition.
- No new simulation command type unless explicitly approved.

Acceptance criteria:
- `engage contact <id>` emits `EngageContactCommand` for the currently selected friendly entity.
- The command is rejected by UI feedback if no visible friendly entity is selected.
- The command is rejected by UI feedback if the contact ID is not visible in the tactical snapshot.
- Help text and `docs/contracts/ui_commands.md` include the new command.
- Integration tests cover successful command emission and both UI rejection paths.

Deliverables:
- Implementation.
- Tests.
- Updated UI command contract.
- Completion report.

Out of scope:
- Salvos.
- Engagement doctrine.
- Targeting hidden entity IDs from UI.
- Simulation launch behavior changes.

## SCENARIO-002 — Playable engagement demo scenario

Task:
SCENARIO-002 — Playable engagement demo scenario

Objective:
Add a dedicated demo scenario that starts with enough information and ammunition
to exercise contact inspection, engagement, missile approach, defensive response,
events, and time-scale recommendations through `scs_demo`.

Relevant files:
- `src/simulation/scenario.*`
- `src/app/main.cpp`
- `tests/simulation/deterministic_replay_tests.cpp`
- `tests/integration/vertical_slice_replay_tests.cpp`
- `docs/mechanics/scenario.md`

Allowed files:
- `src/simulation/scenario.*`
- `src/app/main.cpp`
- `tests/simulation/*`
- `tests/integration/*`
- `docs/mechanics/scenario.md`
- `CMakeLists.txt`, only if a new scenario test target is needed

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/contracts/contact_snapshot.md`
- `docs/contracts/engagement_commands.md`
- `docs/mechanics/scenario.md`

Invariants:
- Preserve the existing regression scenario unless the owner explicitly approves replacing it.
- The demo scenario must be deterministic.
- No scenario file loading or procedural generation.
- No new gameplay systems.

Acceptance criteria:
- A new scenario factory creates a playable two-group engagement setup.
- The demo scenario includes a visible friendly group, a visible hostile contact, at least one friendly missile, and at least one hostile defensive response charge.
- `scs_demo` starts in the playable scenario by default.
- Tests verify initial contact visibility, ammunition/defense values, and replay determinism.
- Mechanics docs distinguish the regression scenario from the playable demo scenario.

Deliverables:
- Implementation.
- Tests.
- Updated scenario mechanics doc.
- Completion report.

Out of scope:
- Scenario selection menu.
- JSON scenario loading.
- Additional factions or groups.
- Economy, campaign, or procedural content.

## UIEVAL-001 — UI stack decision after enriched console

Task:
UIEVAL-001 — UI stack decision after enriched console

Objective:
Evaluate the next UI stack only after the console map includes tactical metrics,
missile display, and engagement command flow. Record the follow-up selection
model questions for the chosen UI direction, but do not implement graphical
interaction in this task.

Relevant files:
- `docs/decisions/0002_ui_stack.md`
- `docs/decisions/0003_ui_evolution_options.md`
- `docs/contracts/tactical_display_metrics.md`

Allowed files:
- `docs/decisions/*`
- `docs/contracts/tactical_display_metrics.md`

Required context:
- `AGENTS.md`
- `docs/decisions/0002_ui_stack.md`
- `docs/decisions/0003_ui_evolution_options.md`
- completion reports for `PRESENT-002`, `DISPLAY-001`, `UI-002`, and `SCENARIO-002`

Invariants:
- This is a decision task, not an implementation task.
- Do not introduce a dependency.
- Do not change build tooling.
- Do not remove the console UI until a replacement is implemented and tested.

Acceptance criteria:
- Compare improved console, terminal TUI, desktop SFML/ImGui, and web UI against the decision criteria in `0003_ui_evolution_options.md`.
- Recommend one next UI stack or recommend staying console for one more pass.
- List owner decisions required before UI implementation, including the
  object-selection model.
- Update `0003_ui_evolution_options.md` status and decision text.

Deliverables:
- Updated ADR.
- Short completion report.

Out of scope:
- UI implementation.
- Dependency installation.
- Renderer rewrite.
- Packaging work.
- Graphical interaction or selection-model implementation.

---

## Desktop UI migration round

The accepted next UI stack is optional desktop SFML/Dear ImGui. The console
demo remains the fallback and regression surface until the desktop path is
playable and tested.

Recommended sequence:

1. `UIDESKTOP-001` — gated desktop dependency and target.
2. `UIDESKTOP-002` — pure desktop map interaction.
3. `UIDESKTOP-003` — SFML tactical map renderer.
4. `UIDESKTOP-004` — Dear ImGui panels and command emission.
5. `UIDESKTOP-005` — desktop app integration and smoke.

## UIDESKTOP-001 — Gated desktop dependency and target

Task:
UIDESKTOP-001 — Gated desktop dependency and target

Objective:
Add the optional SFML/Dear ImGui build path without changing the default build
or replacing the console demo.

Relevant files:
- `CMakeLists.txt`
- `src/app/main.cpp`
- `docs/decisions/0003_ui_evolution_options.md`

Allowed files:
- `CMakeLists.txt`
- `cmake/*`
- `src/app/desktop_main.cpp`
- `docs/decisions/0003_ui_evolution_options.md`
- `docs/contracts/desktop_ui.md`

Required context:
- `AGENTS.md`
- `docs/decisions/0002_ui_stack.md`
- `docs/decisions/0003_ui_evolution_options.md`

Invariants:
- Default build remains dependency-free.
- `scs_core` and default `scs_ui` do not include SFML or ImGui headers.
- Console `scs_demo` remains unchanged.
- Do not upgrade to SFML 3.

Acceptance criteria:
- Default build and all existing tests still pass.
- `-DSCS_BUILD_DESKTOP_UI=ON` configures.
- `scs_desktop` builds.
- Dependency pins and enable command are documented in `docs/contracts/desktop_ui.md`.

Deliverables:
- Build changes.
- Minimal desktop executable.
- Desktop UI contract update.
- Completion report.

Out of scope:
- Tactical rendering.
- Command panels.
- Packaging.
- Replacing console UI.

## UIDESKTOP-002 — Pure desktop map interaction

Task:
UIDESKTOP-002 — Pure desktop map interaction

Objective:
Implement testable map projection, hit testing, hover, nearest-object selection,
overlap cycling, and staged engagement state without SFML or ImGui dependencies.

Relevant files:
- `src/rendering/tactical_map_renderer.*`
- `src/ui/tactical_command_ui.*`
- `src/presentation/tactical_snapshot.*`

Allowed files:
- `src/ui/desktop_interaction.*`
- `src/rendering/tactical_map_projection.*`
- `tests/integration/*`
- `docs/contracts/desktop_ui.md`
- `CMakeLists.txt`, only to add tests

Required context:
- `AGENTS.md`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/ui_commands.md`
- `docs/contracts/tactical_display_metrics.md`

Invariants:
- Consume only `presentation::TacticalSnapshot` plus UI/display state.
- Do not query simulation.
- Do not expose hidden hostile entities.
- Missiles are hover/inspector-only, not selectable.
- No new command types.

Acceptance criteria:
- Tests cover world/screen projection, pan, zoom around cursor, nearest
  friendly/contact selection, overlap cycling, hidden-data absence, missile
  hover-only behavior, and staged engage command emission.

Deliverables:
- Pure interaction code.
- Tests.
- Contract update.
- Completion report.

Out of scope:
- SFML drawing.
- ImGui widgets.
- New simulation commands.

## UIDESKTOP-003 — SFML tactical map renderer

Task:
UIDESKTOP-003 — SFML tactical map renderer

Objective:
Draw the tactical snapshot with SFML: grid/reference space, friendly groups,
hostile contacts, uncertainty circles, predicted trajectories, missile tracks,
selection, and hover emphasis.

Relevant files:
- `src/rendering/tactical_map_renderer.*`
- `src/presentation/tactical_snapshot.*`

Allowed files:
- `src/rendering/sfml_tactical_map_renderer.*`
- `src/rendering/tactical_map_projection.*`
- `tests/integration/*`
- `docs/contracts/desktop_ui.md`
- `CMakeLists.txt`

Required context:
- `AGENTS.md`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/tactical_display_metrics.md`
- `docs/contracts/desktop_ui.md`

Invariants:
- Rendering consumes presentation data only.
- No simulation mutation.
- No ImGui panel logic.
- No gameplay rules in rendering.

Acceptance criteria:
- Desktop build renders map primitives.
- Projection matches UIDESKTOP-002 tests.
- Uncertainty and missile tracks use player-facing snapshot fields only.
- Existing console renderer tests still pass.

Deliverables:
- SFML renderer.
- Focused projection/render smoke tests where headless-safe.
- Completion report.

Out of scope:
- Panel layout.
- Command emission.
- Visual polish beyond readability.

## UIDESKTOP-004 — Dear ImGui panels and command emission

Task:
UIDESKTOP-004 — Dear ImGui panels and command emission

Objective:
Add ImGui panels for selection inspector, hover inspector, event log, command
log, pause/resume, step/run, scale override, and staged engage.

Relevant files:
- `src/ui/tactical_command_ui.*`
- `src/domain/command.h`
- `src/gameplay/time_scale_policy.*`

Allowed files:
- `src/ui/imgui_tactical_panels.*`
- `src/ui/desktop_interaction.*`
- `tests/integration/*`
- `docs/contracts/desktop_ui.md`
- `docs/contracts/ui_commands.md`

Required context:
- `AGENTS.md`
- `docs/contracts/ui_commands.md`
- `docs/contracts/time_scale_policy.md`
- `docs/contracts/engagement_commands.md`

Invariants:
- UI emits `domain::Command` values only.
- Simulation validates commands.
- No new domain command type.
- Tactical pause and manual scale remain app/player state.

Acceptance criteria:
- Tests cover staged engage command emission and invalid staged engagement.
- Existing tests cover pause/resume, manual scale, and step/run behavior.
- No UI path directly mutates simulation.

Deliverables:
- ImGui panel code.
- Pure command-emission tests.
- Contract updates.
- Completion report.

Out of scope:
- Right-click command menus.
- Command palette.
- Selectable missiles.

## UIDESKTOP-005 — Desktop app integration and smoke

Task:
UIDESKTOP-005 — Desktop app integration and smoke

Objective:
Wire the desktop executable into the playable engagement scenario while keeping
simulation advancement fixed-tick and independent of render frame rate.

Relevant files:
- `src/app/main.cpp`
- `src/app/desktop_main.cpp`
- `src/simulation/scenario.*`

Allowed files:
- `src/app/desktop_main.cpp`
- `docs/contracts/desktop_ui.md`
- `docs/mechanics/scenario.md`
- `CMakeLists.txt`, only if target wiring needs adjustment

Required context:
- `AGENTS.md`
- `docs/contracts/simulation_core.md`
- `docs/contracts/tactical_snapshot.md`
- `docs/contracts/time_scale_policy.md`
- `docs/contracts/desktop_ui.md`

Invariants:
- Render frame time never changes simulation state directly.
- Simulation advances only through explicit fixed ticks.
- Console demo remains available.
- Replay determinism remains covered by existing tests.

Acceptance criteria:
- Desktop app starts in the playable engagement scenario.
- User can pan, zoom, hover, select friendly/contact, pause/resume, step/run,
  see time-scale reason, see event log, and issue engage contact.
- All existing tests pass.
- Manual smoke notes are recorded in the completion report.

Deliverables:
- Integrated desktop app.
- Smoke notes.
- Completion report.

Out of scope:
- Packaging.
- Save/load.
- Multiplayer.
- 3D.
- Campaign UI.
