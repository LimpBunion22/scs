#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "domain/contact.h"
#include "domain/entity.h"
#include "domain/time.h"

namespace scs::domain {

struct MissileId {
    std::uint32_t value{0};

    constexpr MissileId() = default;
    explicit constexpr MissileId(std::uint32_t in_value) : value(in_value) {}
};

constexpr bool operator==(MissileId lhs, MissileId rhs) {
    return lhs.value == rhs.value;
}

constexpr bool operator!=(MissileId lhs, MissileId rhs) {
    return !(lhs == rhs);
}

constexpr bool operator<(MissileId lhs, MissileId rhs) {
    return lhs.value < rhs.value;
}

constexpr bool is_valid(MissileId id) {
    return id.value != 0;
}

enum class MissileStatus {
    InFlight,
    Hit,
    Miss,
    Defeated
};

struct EntitySnapshot {
    EntityId id;
    EntityKind kind{EntityKind::CombatGroup};
    Allegiance allegiance{Allegiance::Unknown};
    std::string name;
    Vec2 position_km;
    Vec2 velocity_km_per_second;
    int missile_ammunition{0};
    int defensive_response_charges{0};
};

struct MissileSnapshot {
    MissileId id;
    EntityId launcher;
    EntityId target_entity;
    ContactId target_contact;
    Vec2 position_km;
    Vec2 velocity_km_per_second;
    MissileStatus status{MissileStatus::InFlight};
};

struct WorldSnapshot {
    Tick tick{0};
    double time_seconds{0.0};
    std::vector<EntitySnapshot> entities;
    std::vector<ContactSnapshot> contacts;
    std::vector<MissileSnapshot> missiles;
};

} // namespace scs::domain
