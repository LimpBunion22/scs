# Missiles

The first missile slice implements one missile type and one deterministic defensive
response.

## State

- Combat groups carry explicit `missile_ammunition`.
- Combat groups carry explicit `defensive_response_charges`.
- Missile tracks are included in world snapshots with ID, launcher, target entity,
  optional target contact, position, velocity, and status.

## Flight

- Missile speed is `100 km/s`.
- Movement advances only on fixed simulation ticks.
- A missile steers directly toward the target entity's current position.
- A `MissileThreat` event fires once when the missile is within `250 km`.
- A missile hits when it reaches the target within `1 km`.

## Defensive Response

- A target with a defensive response charge defeats one incoming missile within
  `100 km`.
- The charge is consumed deterministically.
- A defeated missile records `Defeated` status and emits both `DefensiveResponse`
  and `MissileMissed`.

No component damage, salvos, multiple weapon families, or probabilistic resolution
are implemented.
