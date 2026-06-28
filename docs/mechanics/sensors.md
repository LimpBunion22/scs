# Simplified Sensors

The first sensor model is deterministic and deliberately small.

## Detection

- Each entity has a circular sensor range in kilometers.
- Friendly and hostile entities can observe each other when the target is inside
  the observer's sensor range.
- Neutral and unknown allegiances do not create tracks in this slice.
- Sensor checks run at scenario load for tick `0` and after each fixed movement tick.
- Contact update order is stable by observer ID, then target ID.

## Contact Estimates

- A fresh observation copies the target position and velocity into the contact estimate.
- A fresh observation sets confidence to `1.0`.
- A fresh observation sets uncertainty radius to `0 km`.
- A stale contact remains in the snapshot after the target leaves sensor range.
- Each stale tick projects the estimate by the last observed velocity.
- Stale uncertainty grows by `100 km` per simulated second.
- Stale confidence decays by `0.05` per simulated second and is clamped at `0.0`.

## Events

- First detection emits `ContactDetected` with `Advisory` severity.
- Refreshing an existing observed contact emits `ContactUpdated` with `Info` severity.
- Stale projection does not emit an event.
