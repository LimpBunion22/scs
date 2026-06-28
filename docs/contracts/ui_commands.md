# UI Command Contract

The first tactical UI is a line-command console surface. It owns UI state and
emits `domain::Command` values; it does not mutate simulation entities directly.

## Inputs

- `presentation::TacticalSnapshot`
- current UI state:
  - map center in kilometers;
  - kilometers per map cell;
  - selection;
  - tactical pause flag;
  - optional manual time-scale override.

## Commands

| Command | Effect |
| --- | --- |
| `pan <east_cells> <north_cells>` | Moves the map center by view-cell offsets. |
| `zoom in` | Halves kilometers per cell, clamped to the UI minimum. |
| `zoom out` | Doubles kilometers per cell, clamped to the UI maximum. |
| `zoom <km_per_cell>` | Sets map scale directly. |
| `select friendly <id>` | Selects a visible friendly entity. |
| `select contact <id>` | Selects a visible hostile contact. |
| `select none` | Clears selection. |
| `pause` | Sets tactical pause in UI/player time-control state. |
| `resume` | Clears tactical pause. |
| `scale auto` | Clears manual time-scale override. |
| `scale <multiplier>` | Sets manual time-scale override. |
| `run [ticks]` | Requests explicit tick advancement while not paused. |
| `step [ticks]` | Requests explicit tick advancement even while paused. |
| `velocity <vx> <vy>` | Emits `SetVelocityCommand` for the selected friendly entity at the current snapshot tick. |
| `engage contact <id>` | Emits `EngageContactCommand` for the selected friendly entity and visible hostile contact at the current snapshot tick. |
| `quit` | Requests app shutdown. |

The desktop ImGui panels must preserve the same command semantics:

- `Set Velocity` emits `SetVelocityCommand` at the current snapshot tick.
- The target is the currently selected visible friendly entity, or the staged
  visible friendly entity when the current inspector selection is not a
  friendly.
- Desktop maneuver emission is rejected when no visible friendly entity is
  selected or staged.

## Invariants

- Command emission is data-only. The app decides whether to submit emitted
  commands to simulation.
- Rendering uses `presentation::TacticalSnapshot`; it does not query mutable
  simulation state.
- Tactical pause and manual scale are app/player inputs to
  `gameplay::recommend_time_scale`.
- Simulation advancement remains explicit tick advancement requested by app/UI
  control flow.
- Engagement command validation beyond visible selected friendly and visible
  contact remains the simulation's responsibility.
- Velocity command validation beyond visible selected or staged friendly remains
  the simulation's responsibility.
