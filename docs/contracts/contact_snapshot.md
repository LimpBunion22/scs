# Contact Snapshot Contract

Contact snapshots are read-only estimates produced by simulation sensors. A contact
does not expose the target entity ID.

## Fields

| Field | Meaning |
| --- | --- |
| `id` | Stable contact ID assigned deterministically on first detection. |
| `observer` | Entity ID of the entity that owns the track. |
| `estimated_position_km` | Estimated contact position in kilometers. |
| `estimated_velocity_km_per_second` | Estimated contact velocity in kilometers per second. |
| `last_observed_tick` | Last simulation tick with a direct sensor observation. |
| `confidence` | Normalized track confidence from `0.0` to `1.0`. |
| `classification` | Current simplified classification. The first slice uses `HostileCombatGroup`. |
| `uncertainty_radius_km` | Radius around the estimated position in kilometers. |

## Ordering

Contacts are created in stable observer-ID, then target-ID order. Snapshots preserve
contact creation order.
