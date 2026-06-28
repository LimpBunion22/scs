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
| `hostile_contacts` | Contact estimates copied from `WorldSnapshot::contacts` only when the contact observer is present in `friendly_entities`. Contacts remain estimates and do not expose target entity IDs. |
| `missile_tracks` | Player-visible missile tracks copied from friendly-launched `WorldSnapshot::missiles`. Tracks expose missile ID, launcher ID, target contact ID when known, position, velocity, and status. They do not expose hidden target entity IDs. |
| `events` | Event stream copied in source order for command/event log presentation. |
| `predicted_trajectories` | Deterministic inertial trajectories for friendly entities and hostile contacts. |

## Visibility Rules

- A visible friendly entity is any `WorldSnapshot::entities` entry with
  `Allegiance::Friendly`.
- `friendly_entities` contains only visible friendly entities.
- `hostile_contacts` contains only contacts whose `observer` matches a visible
  friendly entity. Contacts owned by hidden hostile, neutral, unknown, or absent
  observers are omitted.
- `missile_tracks` contains only missiles whose `launcher` matches a visible
  friendly entity.
- Missile tracks intentionally omit `target_entity`; player-facing display may
  use `target_contact` when it is populated, but must not recover hidden hostile
  entity truth through tactical presentation.

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
- Hostile contacts preserve `WorldSnapshot::contacts` order after filtering.
- Missile tracks preserve `WorldSnapshot::missiles` order after filtering.
- Events preserve source event order.
- Predicted trajectories are ordered as friendly entity trajectories followed by
  hostile contact trajectories.
