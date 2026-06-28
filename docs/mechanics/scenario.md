# Core Scenario

The default vertical-slice scenario is the deterministic baseline for simulation, replay,
sensor, contact, missile, and presentation work.

## Units

- Position: kilometers.
- Velocity: kilometers per second.
- Sensor range: kilometers.
- Time: fixed simulation ticks, with the default scenario using one second per tick.

## Baseline

The scenario is named `vertical_slice_core` and uses seed `0x5c5c0001`.
It contains exactly two combat groups:

| ID | Name | Allegiance | Position km | Velocity km/s | Sensor range km |
| --- | --- | --- | --- | --- | --- |
| 1 | Blue Command Group | Friendly | `(-1000000, 0)` | `(18, 0)` | `750000` |
| 2 | Red Command Group | Hostile | `(1000000, 150000)` | `(-16, -0.25)` | `650000` |

## Intent

The groups begin far apart and move on deterministic inertial tracks. The opening
distance keeps sensors, uncertain contacts, and missile engagement as later mechanics
rather than implicit behavior in the core scenario setup.
