#pragma once

#include <string>

#include "domain/ids.h"
#include "domain/vector2.h"

namespace scs::domain {

enum class Allegiance {
    Unknown,
    Friendly,
    Hostile,
    Neutral
};

enum class EntityKind {
    CombatGroup
};

struct EntityState {
    EntityId id;
    EntityKind kind{EntityKind::CombatGroup};
    Allegiance allegiance{Allegiance::Unknown};
    std::string name;
    Vec2 position_km;
    Vec2 velocity_km_per_second;
    double sensor_range_km{0.0};
};

} // namespace scs::domain
