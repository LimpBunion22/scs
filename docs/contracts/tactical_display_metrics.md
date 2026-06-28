# Tactical Display Metrics Contract

This contract defines the next tactical-map display layer. It is derived from
presentation snapshots, UI selection, and map view state. It must not query or
mutate simulation internals.

## Purpose

The map must provide reference and motion information, not only object markers.
The first implementation may remain text based, but it must expose enough
numbers for the player to understand scale, motion, contact quality, and weapon
timing.

## Inputs

- `presentation::TacticalSnapshot`
- `rendering::TacticalMapView`
- `rendering::TacticalSelection`
- optional `gameplay::TimeScaleRecommendation`

## Reference Frame

The display uses the existing world coordinate frame:

- `+x`: east, kilometers;
- `+y`: north, kilometers;
- velocity: kilometers per second;
- bearing: degrees clockwise from north;
- range: kilometers.

The map should always show:

- simulation tick and elapsed seconds;
- map center in kilometers;
- kilometers per cell;
- visible span in kilometers;
- orientation labels for east and north.

## Selection Metrics

When a friendly entity is selected, show:

- entity ID and name;
- position;
- velocity vector;
- speed;
- ammunition and defensive response charges.

When a hostile contact is selected, show:

- contact ID and observer ID;
- estimated position;
- estimated velocity;
- confidence;
- uncertainty radius;
- age in ticks since last observation;
- range and bearing from its observer when that observer is visible;
- closing speed relative to its observer;
- deterministic time and distance to closest approach relative to its observer.

Closest approach is computed from current snapshot estimates:

- relative position = contact estimated position minus observer position;
- relative velocity = contact estimated velocity minus observer velocity;
- if relative speed is zero, closest-approach time is `0`;
- otherwise closest-approach time is `max(0, -dot(relative_position, relative_velocity) / dot(relative_velocity, relative_velocity))`.

## Missile Tracks

When missile tracks are available in the tactical snapshot, show:

- missile ID;
- launcher ID;
- target contact ID when available;
- position;
- velocity;
- speed;
- status.

The display must not reveal hostile entity state that the presentation layer has
not exposed. A missile may be associated with a hidden target internally, but the
player-facing display should prefer the contact ID and estimated track.

## Event and Time Context

The event log should keep severity, tick, type, subject, and message visible.
If a time-scale recommendation is provided, show its multiplier and reason near
the map header.

## Invariants

- Metrics are deterministic pure derivations from display inputs.
- Rendering does not submit commands.
- Rendering does not advance simulation.
- Rendering does not access mutable simulation state.
- Missing observer/contact data must produce an explicit `unknown` or omitted
  metric, not fabricated numbers.
