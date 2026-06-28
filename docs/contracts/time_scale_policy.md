# Time Scale Policy Contract

The time-scale policy is gameplay code. It recommends an app-visible simulation
speed multiplier from explicit event data and player/app time-control state.

## Inputs

- `current_tick`: the simulation tick used as the evaluation point.
- `events`: tick-stamped `domain::Event` values already emitted by simulation.
- `PlayerTimeScaleInput`: explicit app/player state for tactical pause and
  manual scale override.
- `TimeScalePolicyConfig`: deterministic mapping and recent-event window.

The policy does not read wall-clock time and does not advance simulation state.

## Recent Events

By default, events are recent when:

- `event.tick <= current_tick`; and
- `current_tick - event.tick <= 30`.

The policy uses the highest severity among recent events. Future events and
events older than the configured window are ignored.

## Default Mapping

| Input state | Recommended scale | Reason |
| --- | ---: | --- |
| Tactical pause | `0x` | `TacticalPause` |
| Player override | override value | `PlayerOverride` |
| No recent events | `1024x` | `Quiet` |
| Recent `Info` | `256x` | `RecentInfo` |
| Recent `Advisory` | `64x` | `RecentAdvisory` |
| Recent `Threat` | `8x` | `RecentThreat` |
| Recent `Critical` | `1x` | `RecentCritical` |

Tactical pause and player override are explicit inputs. The simulation core
remains responsible only for fixed-tick advancement.

## Output

`TimeScaleRecommendation` contains:

- `scale`: a deterministic multiplier recommendation; and
- `reason`: a `TimeScaleReason` enum suitable for UI display.

`time_scale_reason_label` provides a stable short label for each reason.
