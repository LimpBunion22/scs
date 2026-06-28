# Tactical Snapshot Contract

The presentation layer converts read-only simulation snapshots and event streams
into tactical-map data. It must not mutate simulation state or query mutable
simulation internals.

## Inputs

- `domain::WorldSnapshot`
- ordered `domain::Event` stream
- `presentation::TacticalSnapshotOptions`

## Fields

| Field | Meaning |
| --- | --- |
| `tick` | Simulation tick copied from the world snapshot. |
| `time_seconds` | Simulation time copied from the world snapshot. |
| `friendly_entities` | Friendly entity snapshots copied from `WorldSnapshot::entities`. Hostile entity snapshots are not exposed here. |
| `hostile_contacts` | Contact estimates copied from `WorldSnapshot::contacts`. Contacts remain estimates and do not expose target entity IDs. |
| `events` | Event stream copied in source order for command/event log presentation. |
| `predicted_trajectories` | Deterministic inertial trajectories for friendly entities and hostile contacts. |

## Predicted Trajectories

Each trajectory records:

- `source_kind`: `FriendlyEntity` or `HostileContact`;
- `entity`: populated only for friendly entity trajectories;
- `contact`: populated only for hostile contact trajectories;
- `points`: current position plus one point per future tick through the configured horizon.

Positions remain in kilometers. Velocities are interpreted as kilometers per
second. The first slice infers seconds per tick from `time_seconds / tick` when
`tick > 0`; at tick zero it assumes the current one-second fixed tick used by
the vertical-slice scenario. If non-1s initial projections become required,
`WorldSnapshot` should expose fixed-step duration explicitly.

## Ordering

- Friendly entities preserve `WorldSnapshot::entities` order after filtering.
- Hostile contacts preserve `WorldSnapshot::contacts` order.
- Events preserve source event order.
- Predicted trajectories are ordered as friendly entity trajectories followed by
  hostile contact trajectories.
