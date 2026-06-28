#pragma once

namespace scs::domain {

struct Vec2 {
    double x{0.0};
    double y{0.0};
};

constexpr Vec2 operator+(Vec2 lhs, Vec2 rhs) {
    return Vec2{lhs.x + rhs.x, lhs.y + rhs.y};
}

constexpr Vec2 operator-(Vec2 lhs, Vec2 rhs) {
    return Vec2{lhs.x - rhs.x, lhs.y - rhs.y};
}

constexpr Vec2 operator*(Vec2 value, double scalar) {
    return Vec2{value.x * scalar, value.y * scalar};
}

constexpr Vec2 operator*(double scalar, Vec2 value) {
    return value * scalar;
}

inline Vec2& operator+=(Vec2& lhs, Vec2 rhs) {
    lhs = lhs + rhs;
    return lhs;
}

constexpr double magnitude_squared(Vec2 value) {
    return value.x * value.x + value.y * value.y;
}

} // namespace scs::domain
