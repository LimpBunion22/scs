#include "simulation/missile.h"

namespace scs::simulation {

domain::MissileSnapshot make_missile_snapshot(const MissileState& missile) {
    return domain::MissileSnapshot{
        missile.id,
        missile.launcher,
        missile.target_entity,
        missile.target_contact,
        missile.position_km,
        missile.velocity_km_per_second,
        missile.status,
    };
}

} // namespace scs::simulation
