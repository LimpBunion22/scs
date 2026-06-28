#pragma once

#include <cstdint>

namespace scs::domain {

struct EntityId {
    std::uint32_t value{0};

    constexpr EntityId() = default;
    explicit constexpr EntityId(std::uint32_t in_value) : value(in_value) {}
};

constexpr bool operator==(EntityId lhs, EntityId rhs) {
    return lhs.value == rhs.value;
}

constexpr bool operator!=(EntityId lhs, EntityId rhs) {
    return !(lhs == rhs);
}

constexpr bool operator<(EntityId lhs, EntityId rhs) {
    return lhs.value < rhs.value;
}

constexpr bool is_valid(EntityId id) {
    return id.value != 0;
}

} // namespace scs::domain
