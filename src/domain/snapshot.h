#pragma once

#include <string>
#include <vector>

#include "domain/entity.h"
#include "domain/time.h"

namespace scs::domain {

struct EntitySnapshot {
    EntityId id;
    EntityKind kind{EntityKind::CombatGroup};
    Allegiance allegiance{Allegiance::Unknown};
    std::string name;
    Vec2 position_km;
    Vec2 velocity_km_per_second;
};

struct WorldSnapshot {
    Tick tick{0};
    double time_seconds{0.0};
    std::vector<EntitySnapshot> entities;
};

} // namespace scs::domain
