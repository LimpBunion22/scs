# AGENTS.md

## Project: Command-Level Space Combat Strategy Game

This repository contains a space combat strategy game focused on command, information, timing, and uncertainty rather than direct ship control.

This file is the authoritative starting point for Codex and any other coding agents working on the repository.

---

## 1. Product Vision

The game should simulate command-level space combat across very large spatial and temporal scales.

The core experience is based on:

- commanding fleets rather than piloting ships;
- incomplete and uncertain information;
- delayed detection and imperfect target tracking;
- choosing when to reveal position;
- deciding when to engage, disengage, maneuver, or conserve ammunition;
- automatic adaptation of simulation speed to the density of meaningful decisions;
- deterministic simulation and replay;
- a tactical command map as the primary interface;
- continuous transitions between operational and tactical levels of detail.

The game must not become:

- an arcade RTS;
- a direct ship piloting simulator;
- a graphics-first project without validated gameplay;
- a large physical simulation with no immediate gameplay value;
- a collection of speculative systems implemented “for later”.

Every implemented system must support a concrete interaction in the next playable prototype.

---

## 2. Current Goal

The immediate goal is not to build the complete game.

The immediate goal is to produce the smallest playable vertical slice that validates the following ideas:

1. Large distances can be represented clearly.
2. Time acceleration can adapt automatically to relevant events.
3. The player can understand uncertain contacts.
4. Orders can be issued without directly controlling ships.
5. Missile engagement can create meaningful decisions.
6. The interface can remain readable without overwhelming the player.

The first prototype should include only:

- a 2D tactical map;
- two opposing combat groups;
- basic inertial movement;
- simplified sensors;
- uncertain contacts;
- one missile type;
- one defensive response;
- event-based automatic time scaling;
- tactical pause;
- a command/event log;
- deterministic replay.

Do not add economy, campaign systems, ship construction, technology trees, detailed crew simulation, multiple weapon families, 3D rendering, multiplayer, or procedural content unless explicitly requested.

---

## 3. Operating Model for Agents

Agents must work as specialized, disposable workers operating against explicit contracts.

No agent should assume that another agent shares its conversation history.

The repository is the shared memory.

Important decisions must be written into the repository rather than left only in chat.

Agents must minimize context consumption by reading only:

- this file;
- the files directly relevant to the assigned task;
- the interfaces imported by the code being modified;
- the tests covering the affected behavior.

Do not recursively inspect the whole repository unless the task is architectural or explicitly requests a full audit.

---

## 4. Authority and Decision Rules

The human owner and the main directing conversation define product direction.

Agents may make local implementation decisions when they:

- do not alter public contracts;
- do not introduce new dependencies;
- do not change gameplay semantics;
- do not increase project scope;
- do not conflict with this document.

Agents must stop and report a pending decision when a task requires:

- changing a public interface;
- modifying the simulation model;
- introducing a third-party library;
- changing build tooling;
- changing save or replay formats;
- altering gameplay behavior outside the assigned task;
- adding a system not required by the current vertical slice.

Do not silently reinterpret ambiguous requirements.

When ambiguity is minor, choose the simplest reversible implementation and document the assumption.

---

## 5. Architectural Principles

The architecture should separate authoritative simulation from presentation.

Preferred high-level structure:

```text
src/
├── domain/
├── simulation/
├── gameplay/
├── presentation/
├── ui/
├── rendering/
└── app/

tests/
├── domain/
├── simulation/
├── gameplay/
└── integration/

docs/
├── decisions/
├── mechanics/
└── contracts/
```

### 5.1 `domain`

Contains stable shared concepts:

- identifiers;
- units;
- timestamps;
- vectors and coordinates;
- commands;
- events;
- public snapshots;
- enums and value types.

This layer must not depend on SFML, ImGui, or application code.

### 5.2 `simulation`

Contains the authoritative world state and its deterministic evolution:

- entity state;
- movement;
- sensors;
- contact uncertainty;
- weapons;
- impacts;
- event generation;
- time advancement;
- replay inputs.

This layer must not depend on rendering or UI libraries.

### 5.3 `gameplay`

Contains command-level rules and decision logic:

- order validation;
- engagement rules;
- doctrine;
- command delays;
- automatic time-scale policy;
- high-level AI behavior.

### 5.4 `presentation`

Converts simulation state into read-only data suitable for display:

- tactical snapshots;
- predicted trajectories;
- contact confidence;
- alert severity;
- map annotations;
- level-of-detail selection.

Presentation must not mutate the simulation.

### 5.5 `ui`

Contains player interaction:

- panels;
- selection;
- order creation;
- filters;
- overlays;
- timeline controls;
- event log.

The UI emits commands. It must not directly modify ships or simulation entities.

### 5.6 `rendering`

Contains map drawing and visual effects:

- camera;
- grids;
- icons;
- uncertainty regions;
- trajectories;
- weapon tracks;
- labels;
- effects.

Rendering consumes presentation snapshots and does not contain gameplay rules.

### 5.7 `app`

Contains integration:

- main loop;
- dependency wiring;
- state transitions;
- configuration;
- application startup and shutdown.

---

## 6. Core Data Flow

The preferred interaction model is:

```text
Player/UI
   |
   | Command
   v
Simulation
   |
   | Snapshot + Events
   v
Presentation/UI/Rendering
```

Rules:

- Commands are explicit data objects.
- Simulation state is authoritative.
- Snapshots are read-only.
- Events describe meaningful changes.
- Rendering never queries mutable simulation internals directly.
- The same initial state, seed, commands, and time steps must produce the same result.

---

## 7. Determinism

Determinism is a core requirement.

The simulation should support:

- fixed or explicitly represented time steps;
- deterministic random number generation;
- explicit seeds;
- stable ordering of entity updates;
- replay from initial state plus command stream;
- tests comparing replayed outcomes.

Avoid:

- hidden wall-clock dependencies;
- unordered iteration when order affects results;
- random generators without explicit ownership;
- rendering frame rate influencing simulation behavior;
- platform-dependent behavior where practical.

Floating-point determinism across different architectures is not initially required unless a test demonstrates that it is necessary. Within one supported platform and build configuration, replay must remain stable.

---

## 8. Time Model

The game should not require the player to constantly manage fast-forward controls.

Simulation speed should adapt to decision density.

Initial conceptual time levels:

- very high acceleration when no relevant event is near;
- reduced acceleration when a contact is detected or updated;
- further reduction during weapon approach;
- real-time or near-real-time during imminent impact or critical defensive action;
- tactical pause while the player inspects information or creates orders.

The automatic time controller must be:

- predictable;
- visible to the player;
- overrideable;
- based on explicit event severity;
- separated from the simulation state update logic.

Do not encode time scaling as ad hoc UI behavior.

---

## 9. Sensor and Contact Model

The player should not receive perfect enemy state.

A contact should represent an estimate, not the enemy entity itself.

A contact may contain:

- estimated position;
- estimated velocity;
- timestamp of last observation;
- confidence;
- classification;
- uncertainty region;
- possible identity;
- observation source.

Uncertainty should increase when a contact is not observed.

The first implementation may use a simple uncertainty radius or ellipse. It does not need a complete probabilistic tracking filter.

The UI should make a clear distinction between:

- confirmed friendly entities;
- observed hostile entities;
- uncertain contacts;
- predicted positions;
- stale information.

---

## 10. Combat Model

The first combat model should be deliberately small.

Initial engagement:

1. A combat group detects or estimates an enemy contact.
2. The player issues an engagement order.
3. A missile is launched.
4. The missile travels through the simulation.
5. The target may detect the threat.
6. A defensive response may be triggered.
7. The engagement resolves deterministically from explicit state and seeded randomness.
8. The event log explains the outcome.

The interesting decisions should concern:

- whether the contact is reliable enough;
- whether firing reveals position;
- whether the target will leave the intercept envelope;
- whether ammunition should be conserved;
- whether the force should maneuver or disengage;
- when time should slow down.

Do not begin with detailed projectile physics, component damage, thermal systems, or many ammunition types.

---

## 11. Graphics and Interface Direction

The primary view is a tactical command map.

The visual language should prioritize:

- trajectories;
- vectors;
- uncertainty;
- predicted positions;
- threat envelopes;
- timing;
- event severity;
- hierarchy of information.

The map does not need to preserve literal visual scale at every zoom level.

Different zoom levels may change semantic representation:

- operational zoom: combat groups and broad trajectories;
- tactical zoom: formations, contacts, engagement envelopes;
- close tactical zoom: missiles, defensive actions, local geometry.

Icons and uncertainty regions are more important than detailed ship art.

Avoid spending significant time on visual polish before the core loop is proven understandable.

---

## 12. Coding Rules

Follow the existing language standard and project conventions unless explicitly asked to change them.

General rules:

- Prefer small cohesive modules.
- Keep public interfaces minimal.
- Use strong types for identifiers and units when practical.
- Avoid global mutable state.
- Prefer explicit ownership.
- Separate data from rendering.
- Add tests for simulation behavior.
- Do not refactor unrelated code.
- Do not rename broad parts of the project without approval.
- Do not introduce speculative abstractions.
- Do not add dependencies when the standard library or current stack is sufficient.
- Treat warnings as issues to be resolved where practical.
- Preserve buildability after each commit.

When modifying existing code, prefer the smallest coherent change that satisfies the task.

---

## 13. Documentation Rules

Documentation should be short, local, and authoritative.

Create or update documentation only when it records:

- a public contract;
- a gameplay mechanic;
- an architectural decision;
- a non-obvious invariant;
- build or test instructions.

Use:

```text
docs/contracts/
```

for stable interfaces between modules.

Use:

```text
docs/mechanics/
```

for implemented gameplay behavior.

Use:

```text
docs/decisions/
```

for architectural decision records.

Do not create large design documents full of hypothetical future systems.

---

## 14. Task Contract Format

Every implementation task should be defined with:

```text
Task:
Objective:
Relevant files:
Allowed files:
Required context:
Invariants:
Acceptance criteria:
Deliverables:
Out of scope:
```

Example:

```text
Task:
SIM-001 — Contact uncertainty growth

Objective:
Implement deterministic growth of contact uncertainty while the contact is unobserved.

Relevant files:
- src/simulation/sensors/
- src/domain/contact.*

Allowed files:
- src/simulation/sensors/*
- src/domain/contact.*
- tests/simulation/sensors/*

Required context:
- AGENTS.md
- docs/mechanics/sensors.md
- docs/contracts/contact_snapshot.md

Invariants:
- No SFML or ImGui dependencies.
- Same initial state and time interval produce the same result.
- Public interfaces remain unchanged unless explicitly approved.

Acceptance criteria:
- Uncertainty grows monotonically while unobserved.
- A new observation reduces or resets uncertainty.
- Unit tests cover both behaviors.
- Existing tests continue to pass.

Deliverables:
- Implementation.
- Tests.
- Short completion report.

Out of scope:
- Kalman filters.
- Multi-sensor fusion.
- Rendering.
```

---

## 15. Completion Report Format

At the end of every task, return only the information needed for integration:

```text
Completed:
Files changed:
Tests added or updated:
Tests executed:
Assumptions:
Pending decisions:
Risks:
Suggested next task:
```

Do not include a long narration of the work unless explicitly requested.

---

## 16. Git Workflow

Use one branch or worktree per isolated task.

Suggested branch naming:

```text
agent/<area>-<task>
```

Examples:

```text
agent/sim-contact-uncertainty
agent/ui-time-controls
agent/test-deterministic-replay
```

Rules:

- Do not commit directly to the main branch.
- Keep commits scoped to one task.
- Do not mix formatting changes with behavioral changes.
- Run relevant tests before reporting completion.
- Do not merge your own work unless explicitly instructed.
- Report conflicts rather than resolving them through broad unrelated edits.

---

## 17. Multi-Agent Coordination

Parallel agents should work on modules with stable boundaries.

Good parallel tasks:

- inspect simulation determinism;
- prototype contact uncertainty;
- design presentation snapshots;
- implement tactical map camera;
- add replay tests;
- review build structure;
- define event severity and time-scaling rules.

Bad parallel tasks:

- two agents editing the same central header;
- multiple agents redesigning architecture independently;
- one agent changing an interface while another implements against it;
- broad refactors without a frozen contract.

Before assigning parallel work:

1. Freeze the relevant interface.
2. Define allowed files.
3. Define acceptance criteria.
4. Assign one integration owner.
5. Require concise completion reports.

---

## 18. Initial Repository Audit

Before implementing anything, inspect the repository.

Perform the following:

1. Identify the current build system.
2. Identify the current language standard.
3. Map the existing directory structure.
4. Locate the simulation loop.
5. Locate rendering and UI dependencies.
6. Locate existing entity, ship, weapon, and event types.
7. Identify current tests and how they are run.
8. Identify global state and tight coupling.
9. Determine whether simulation depends on frame rate.
10. Determine whether replay or deterministic seeds already exist.
11. Identify code that is reusable for the first vertical slice.
12. Identify code that conflicts with the product vision.
13. Do not modify files during this audit.

Return:

```text
Repository summary:
Build and test commands:
Current architecture:
Reusable components:
Architectural mismatches:
Determinism risks:
First vertical slice proposal:
Recommended task sequence:
Open decisions:
```

---

## 19. First Vertical Slice

After the audit, propose a minimal implementation sequence.

Preferred sequence:

### Phase 1 — Stable simulation shell

- deterministic simulation clock;
- entity identifiers;
- command queue;
- event queue;
- replayable initial scenario;
- simulation independent from render frame rate.

### Phase 2 — Tactical map

- pan and zoom;
- stable world coordinates;
- friendly combat group display;
- predicted trajectory;
- selection and inspection.

### Phase 3 — Uncertain contact

- simplified sensor observation;
- contact track separated from enemy entity;
- uncertainty growth;
- stale-contact visualization;
- contact-related events.

### Phase 4 — Adaptive time

- event severity;
- automatic time-scale recommendations;
- player override;
- tactical pause;
- visible reason for time-scale changes.

### Phase 5 — Missile engagement

- engagement command;
- launch event;
- missile movement;
- intercept prediction;
- one defensive action;
- deterministic resolution;
- event log.

### Phase 6 — Validation

- replay test;
- scenario regression test;
- usability review;
- performance measurement;
- decision on whether the concept justifies expansion.

Do not begin a later phase if the previous phase cannot be demonstrated interactively.

---

## 20. First Instruction to Execute

Start by performing the repository audit described in Section 18.

Do not modify any file yet.

After the audit:

1. propose the smallest vertical slice compatible with the existing code;
2. identify which existing components should be retained, adapted, or removed;
3. produce a task breakdown with explicit file boundaries;
4. list unresolved architectural and gameplay decisions;
5. wait for approval before implementing broad structural changes.

The first response should be concise, concrete, and grounded in the actual repository.
