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

## Next-round boundaries

For the playable tactical-map round:

- run `PRESENT-002` before `DISPLAY-001`, because display metrics depend on
  player-facing contacts and missile tracks;
- `UI-002` can run in parallel with `DISPLAY-001` if it does not change
  presentation or rendering contracts;
- `SCENARIO-002` can run in parallel with `DISPLAY-001` after `PRESENT-002`
  lands, but only one task should edit `src/app/main.cpp`;
- `UIEVAL-001` should wait for completion reports from the implementation
  tasks and should not introduce dependencies;
- graphical interaction and object-selection model implementation must wait
  until `UIEVAL-001` selects the next UI stack.

## Desktop refinement boundaries

After the first desktop SFML/Dear ImGui implementation:

- run `DESKTOPSMOKE-001` first, because it may reveal blockers or owner
  decisions before further UI work;
- `DESKTOPTIME-001` should have one integration owner because it touches
  `src/app/desktop_main.cpp` and the main desktop panel controls;
- after smoke, `DESKTOPPANEL-001` and `DESKTOPRENDER-001` can run in parallel
  if panel work stays in `src/ui/*` and renderer work stays in
  `src/rendering/*`;
- do not run `DESKTOPORDER-001` in parallel with `DESKTOPPANEL-001`, because
  both edit `src/ui/imgui_tactical_panels.*`;
- run `DESKTOPREG-001` after the command and time-control contracts it intends
  to cover have landed;
- only one active task should edit `docs/contracts/desktop_ui.md` at a time, or
  the later task must rebase and preserve prior contract updates.

## Integration rule

If a task needs to change a contract file owned by another active task, stop and report the pending decision instead of guessing.
