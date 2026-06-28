# Simulation core contract

The core simulation is deterministic and independent from UI or rendering.

- Time advances in explicit fixed ticks.
- Commands are data objects submitted to the simulation and executed by tick.
- Commands scheduled for the same tick execute in submission order.
- Entity updates are processed in stable `EntityId` order.
- Snapshots are read-only copies suitable for presentation code.
- Events use simulation ticks, not wall-clock time.
- Replay is defined by scenario data, command stream, and ticks to run.

Initial units:

- position: kilometers;
- velocity: kilometers per second;
- time: seconds represented through fixed ticks.
