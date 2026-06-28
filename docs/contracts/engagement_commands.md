# Engagement Commands Contract

Engagement commands are deterministic simulation commands. The UI or replay stream
submits them as data; simulation validates target state when the command executes.

## Commands

| Command | Subject | Target | Effect |
| --- | --- | --- | --- |
| `EngageEntityCommand` | `launcher` entity ID | `target` entity ID | Launches one missile at a known opposed entity. |
| `EngageContactCommand` | `launcher` entity ID | `target` contact ID | Launches one missile at the entity resolved from that contact estimate. |

## Validation

- The launcher must exist when the command is submitted.
- The command must not be scheduled before the current simulation tick.
- At execution, the launcher must have at least one missile.
- Entity targets must exist and be opposed to the launcher.
- Contact targets must exist and be owned by the launcher.

Accepted commands may still reject at execution if ammunition or target state is no
longer valid.

## Events

Execution emits `MissileLaunched` on success. Invalid execution emits
`CommandRejected`. Missile flight may emit `MissileThreat`, `DefensiveResponse`,
`MissileHit`, and `MissileMissed`.
