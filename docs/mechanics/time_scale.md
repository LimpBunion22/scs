# Adaptive Time Scale

The first vertical slice recommends time acceleration from event severity. The
recommendation is deterministic and advisory; application code decides whether
to apply it.

Default behavior:

- quiet time with no recent event recommends `1024x`;
- recent `Info` events recommend `256x`;
- recent `Advisory` events, including new contact detections, recommend `64x`;
- recent `Threat` events recommend `8x`;
- recent `Critical` events recommend `1x`;
- tactical pause recommends `0x` only when the player/app explicitly requests it;
- a player override uses the requested positive scale until the app clears it.

Recent means an event at the current tick or within the previous `30` ticks by
default. If several recent events exist, the highest severity controls the
recommendation.
