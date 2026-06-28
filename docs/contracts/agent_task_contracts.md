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
