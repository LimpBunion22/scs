# Scenarios

Scenario factories are deterministic C++ setup code. There is no file loading or
procedural generation in the current slice.

## Units

- Position: kilometers.
- Velocity: kilometers per second.
- Sensor range: kilometers.
- Time: fixed simulation ticks, with the default scenario using one second per tick.

## Regression Baseline

The regression baseline is named `vertical_slice_core` and uses seed `0x5c5c0001`.
It contains exactly two combat groups:

| ID | Name | Allegiance | Position km | Velocity km/s | Sensor range km |
| --- | --- | --- | --- | --- | --- |
| 1 | Blue Command Group | Friendly | `(-1000000, 0)` | `(18, 0)` | `750000` |
| 2 | Red Command Group | Hostile | `(1000000, 150000)` | `(-16, -0.25)` | `650000` |

This baseline preserves large-distance inertial movement without implicit contact
or engagement pressure.

## Playable Engagement Demo

The playable console demo is named `playable_engagement_demo` and uses seed
`0x5c5c0002`. It contains exactly two combat groups:

| ID | Name | Allegiance | Position km | Velocity km/s | Sensor range km | Missiles | Defensive charges |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Blue Engagement Group | Friendly | `(0, 0)` | `(0, 0)` | `500` | `1` | `0` |
| 2 | Red Picket Group | Hostile | `(350, 0)` | `(0, 0)` | `0` | `0` | `1` |

Blue observes Red immediately as contact `C1`. In `scs_demo`, the player can run
`engage contact 1` and then advance time to exercise missile launch, threat,
defensive response, defeated-missile events, and time-scale recommendations.
