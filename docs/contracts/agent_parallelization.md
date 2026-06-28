# Agent parallelization notes

Use one branch or worktree per task.

## Stable boundaries

- `src/domain/*` is shared. Avoid parallel edits unless the interface is frozen in a contract.
- `src/simulation/simulation.*` is central. Only one active task should modify it at a time.
- `src/presentation/*`, `src/gameplay/*`, `src/ui/*`, and `src/rendering/*` are intended as separate ownership zones.
- `tests/` may grow by module, but shared test helpers should be introduced only when duplication becomes meaningful.

## Suggested assignment order

First:

- one agent on `SCENARIO-001`;

Then, after `SCENARIO-001` lands:

- one agent on `CONTACT-001`;
- one agent on `TIME-001`;
- optionally one agent on `PRESENT-001`, if it does not depend on contact fields yet.

After contact and presentation contracts are stable:

- one integration owner on `MISSILE-001`;
- one UI owner on `UI-001`.

After the missile loop exists:

- one agent on `REPLAY-001`.

## Integration rule

If a task needs to change a contract file owned by another active task, stop and report the pending decision instead of guessing.
