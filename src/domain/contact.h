#pragma once

#include <cstdint>

#include "domain/ids.h"
#include "domain/time.h"
#include "domain/vector2.h"

namespace scs::domain {

struct ContactId {
    std::uint32_t value{0};

    constexpr ContactId() = default;
    explicit constexpr ContactId(std::uint32_t in_value) : value(in_value) {}
};

constexpr bool operator==(ContactId lhs, ContactId rhs) {
    return lhs.value == rhs.value;
}

constexpr bool operator!=(ContactId lhs, ContactId rhs) {
    return !(lhs == rhs);
}

constexpr bool operator<(ContactId lhs, ContactId rhs) {
    return lhs.value < rhs.value;
}

constexpr bool is_valid(ContactId id) {
    return id.value != 0;
}

enum class ContactClassification {
    Unknown,
    HostileCombatGroup
};

struct ContactSnapshot {
    ContactId id;
    EntityId observer;
    Vec2 estimated_position_km;
    Vec2 estimated_velocity_km_per_second;
    Tick last_observed_tick{0};
    double confidence{0.0};
    ContactClassification classification{ContactClassification::Unknown};
    double uncertainty_radius_km{0.0};
};

} // namespace scs::domain
