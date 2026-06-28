#pragma once

#include "domain/snapshot.h"

namespace scs::simulation {

struct MissileConstants {
    double speed_km_per_second{100.0};
    double threat_range_km{250.0};
    double defense_range_km{100.0};
    double hit_radius_km{1.0};
};

inline constexpr MissileConstants default_missile_constants{};

struct MissileState {
    domain::MissileId id;
    domain::EntityId launcher;
    domain::EntityId target_entity;
    domain::ContactId target_contact;
    domain::Vec2 position_km;
    domain::Vec2 velocity_km_per_second;
    domain::MissileStatus status{domain::MissileStatus::InFlight};
    bool threat_event_emitted{false};
};

[[nodiscard]] domain::MissileSnapshot make_missile_snapshot(const MissileState& missile);

} // namespace scs::simulation
