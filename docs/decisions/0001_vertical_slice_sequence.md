# Decision 0001: first vertical slice sequence

## Status

Accepted for planning.

## Context

The repository now has a small deterministic core:

- fixed simulation ticks;
- replay from scenario, command stream, and tick count;
- entity identifiers;
- command queue;
- simulation event stream;
- read-only world snapshots.

The old SFML/ImGui sandbox has been removed from the active codebase. The next work should validate hidden information, decision timing, and missile engagement before choosing a new UI stack.

## Decision

Build the vertical slice in this order:

1. `SCENARIO-001` — make the core scenario explicit and regression-tested.
2. `CONTACT-001` — add simplified sensors and uncertain contact tracks.
3. `PRESENT-001` — expose read-only tactical presentation snapshots.
4. `TIME-001` — add event-severity based time-scale recommendations.
5. `MISSILE-001` — add one missile engagement and one defensive response.
6. `UI-001` — build the first tactical command map against presentation snapshots.
7. `REPLAY-001` — add a scenario replay regression covering the whole loop.

Do not start `UI-001` until `PRESENT-001` is merged. Do not start `MISSILE-001` until `CONTACT-001` is merged.

## Parallelization

Safe in parallel after `SCENARIO-001`:

- `CONTACT-001`;
- `TIME-001`;
- `PRESENT-001`, if it consumes only existing `WorldSnapshot` fields.

Requires integration ownership:

- `MISSILE-001`, because it changes commands, events, and simulation state;
- `UI-001`, because it may introduce a third-party dependency;
- `REPLAY-001`, because it validates cross-module behavior.

## Consequences

The first playable prototype will stay mechanically small. Systems that do not support contacts, time control, engagement decisions, replay, or the tactical map remain out of scope.
