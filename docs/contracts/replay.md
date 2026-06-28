# Replay Contract

Replay is the deterministic reconstruction of simulation state from explicit
inputs. It does not define a save-file or binary serialization format.

## Inputs

- `Scenario`: name, seed, fixed step, and initial entity state.
- Command stream: ordered `domain::Command` values with explicit execution
  ticks.
- Tick count: the number of fixed simulation ticks to advance.

The replay runner submits the command stream in input order, advances the
simulation by the requested tick count, then returns the final `WorldSnapshot`
and emitted event stream.

## Comparison Rules

Regression tests compare:

- snapshot tick and simulation time;
- entity order and all entity snapshot fields;
- contact order and all contact snapshot fields;
- missile order and all missile snapshot fields;
- event order, tick, severity, type, subject, and message.

Floating-point fields are compared with a small same-build tolerance. Replay
does not currently guarantee bit-identical floating-point results across
architectures.

## Determinism Boundaries

Replay inputs must not depend on UI state, wall-clock time, random devices,
filesystem ordering, or rendering frame rate. If randomness becomes active in a
future mechanic, it must be derived from the replay scenario seed.
