# Desktop Manual Review Checklist

Date: 2026-06-29

Purpose:
This checklist is for owner-side review of the SFML/Dear ImGui desktop app
after the desktop validation/refinement round. Agents have covered deterministic
logic and non-graphical regressions; this pass should focus on what a human can
judge better: readability, ergonomics, visual hierarchy, and whether the command
map feels understandable.

## Setup

Build and run:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

cmake -S . -B build-desktop -DSCS_BUILD_DESKTOP_UI=ON
cmake --build build-desktop --target scs_desktop
./build-desktop/scs_desktop
```

Scenario:
- Default desktop startup should load the playable engagement scenario.

Suggested feedback format:

```text
Item:
Status: PASS / MINOR / BLOCKER / NOT CHECKED
What happened:
Expected instead:
Reproduction steps:
Screenshot: yes/no
Priority: now / later
```

## First Impression

| Item | Status | Notes |
| --- | --- | --- |
| Window opens cleanly | PASS | No crash, no missing font/asset issue, no blank map. |
| First viewport is understandable | PASS | You can tell this is a tactical command map, not a generic debug window. |
| Important state is visible without hunting | PASS | Tick/time, pause/run state, scale reason, selected/staged objects, events. |
| Text density feels usable | PASS | Dense is fine; cramped or clipped is not. |
| No immediate visual noise | PASS | Grid, markers, trajectories, and panels do not compete equally. |

## Layout And Panels

| Item | Status | Notes |
| --- | --- | --- |
| Side panel fits at default window size | PASS | No clipped labels, unreadable values, or controls hidden below the fold. |
| Panel sections are scannable | PASS | Selection, hover, staged command state, missiles, logs, and time controls are easy to distinguish. |
| One metric per row works | PASS | Friendly/contact/missile metrics do not wrap awkwardly or truncate important numbers. |
| Empty states are clear | PASS | `None` or `unknown` states are understandable and not mistaken for errors. |
| Command controls are discoverable | PASS | `Engage Contact` and `Set Velocity` are findable without reading source/docs. |
| Command feedback is visible | PASS | Success/rejection appears in the command log or nearby panel quickly enough. |

## Map Readability

| Item | Status | Notes |
| --- | --- | --- |
| Friendly marker is obvious | PASS | Friendly group is distinct from hostile contacts and missiles. |
| Hostile contact is visually uncertain | PASS | Contact marker plus uncertainty circle communicates estimate, not truth. |
| Missile tracks are visible | PASS | Missile position/track can be seen without overwhelming friendlies/contacts. |
| Predicted trajectories help | I dont see predicted trajectories | Trajectories clarify motion instead of cluttering the map. |
| Grid spacing is useful | PASS | Grid gives scale at default zoom without becoming the main visual feature. |
| Scale bar is readable | Readable, but should include units | Length and label are legible and feel plausible for the current zoom. |
| Off-screen hints are understandable | PASS | Edge hints point toward off-screen objects without looking like selectable objects. |
| Selected object stands out | PASS | Selection emphasis is clearly stronger than normal markers. |
| Hover object stands out | PASS | Hover emphasis is visible but does not look like permanent selection. |

## Interaction

| Item | Status | Notes |
| --- | --- | --- |
| Pan feels predictable | PASS | Right or middle drag moves the map in the expected direction and amount. |
| Zoom feels anchored | PASS | Mouse-wheel zoom keeps the world point under the cursor reasonably stable. |
| Hover updates reliably | PASS | Moving over friendlies, contacts, and missiles updates hover info. |
| Friendlies select reliably | PASS | Left-click selects the friendly group at practical zoom levels. |
| Contacts select reliably | PASS | Left-click selects visible hostile contacts at practical zoom levels. |
| Missiles are hover-only | PASS | Missiles can be inspected by hover but do not become command selections. |
| Overlap cycling feels sane | PASS | If you can create/find overlap, repeated clicks cycle through candidates without surprise. |
| Resize remains usable | PASS | Try default size, smaller window, and larger window. No broken layout. |

## Time Controls

| Item | Status | Notes |
| --- | --- | --- |
| Starts in expected pause/run state | PASS | Current state is visible immediately after startup. |
| Pause stops continuous advancement | PASS | Tick does not keep advancing while paused. |
| Step advances exactly one tick | PASS | Step works while paused and produces one fixed simulation tick. |
| Run advances continuously | PASS | Run mode advances over time without requiring repeated clicks. |
| Auto scale reason is visible |  PASS| Current scale and reason are understandable. |
| Manual scale override is clear | PASS | Selecting a manual scale changes state visibly and can be reverted. |
| No frame-rate weirdness | PASS | Fast/slow rendering does not appear to change simulation semantics. |

## Command Flow

| Item | Status | Notes |
| --- | --- | --- |
| Staging launcher is clear | PASS | Selecting a friendly makes the staged launcher obvious. |
| Staging target is clear | PASS | Selecting a contact makes the staged target obvious. |
| Engage Contact succeeds when staged | PASS | With visible launcher and contact staged, command emits and events follow. |
| Engage Contact rejects clearly when invalid | PASS | Missing launcher/target produces understandable feedback. |
| Set Velocity controls are understandable | PASS | `VX` and `VY` inputs communicate kilometers per second. |
| Set Velocity succeeds for friendly | PASS | Command applies to selected visible friendly, or staged visible launcher. |
| Set Velocity rejects clearly when invalid |  PASS| No visible friendly target produces understandable feedback. |
| Command log is useful | PASS | Log explains what command was attempted and accepted/rejected. |

## Event And Combat Feedback

| Item | Status | Notes |
| --- | --- | --- |
| Initial scenario events make sense |  | Scenario load/contact events are not confusing. |
| Missile launch is visible in log |  | Launch event appears with useful subject/message. |
| Missile threat is visible in log |  | Threat event is easy to notice. |
| Hit/miss/resolution is understandable |  | Outcome is visible and explainable from the log and map. |
| Event severity reads correctly |  | Critical/threat/advisory/info states are distinguishable enough. |
| Time-scale response feels appropriate |  | App slows or recommends scale in a way that matches event importance. |

## Specific Regression Checks

| Item | Status | Notes |
| --- | --- | --- |
| No hidden hostile truth leaks |  | UI should show contacts/estimates, not perfect hostile entity truth. |
| Unknown values are acceptable |  | Missing observer/target data should show `unknown`, not fabricated numbers. |
| Console fallback still exists |  | Desktop work should not remove or replace the console demo. |
| Default tests still pass |  | If you rebuild after local changes, default CTest should pass. |
| Desktop build still works |  | `scs_desktop` should compile with `SCS_BUILD_DESKTOP_UI=ON`. |

## Quick Review Path

Use this if you want a fast pass before filing feedback:

1. Start `scs_desktop`.
2. Resize the window smaller and larger.
3. Pan and zoom around the friendly and hostile contact.
4. Hover friendly, contact, and missile if one is visible.
5. Select friendly, then contact; confirm staged launcher/target.
6. Use `Step`, `Run`, `Pause`, manual scale, and auto scale.
7. Fire `Engage Contact` and watch map, event log, and command log.
8. Set a new velocity with `VX`/`VY` and confirm feedback.
9. Note any clipped text, ambiguous state, unreadable marker, or surprising command behavior.

## Feedback Notes

Add notes below or paste them back into chat.

```text
Overall:

Blockers:

Minor issues:

Visual/readability tuning:

Interaction tuning:

Questions:
```
