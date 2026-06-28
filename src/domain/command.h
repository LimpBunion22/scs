#pragma once

#include <variant>

#include "domain/ids.h"
#include "domain/time.h"
#include "domain/vector2.h"

namespace scs::domain {

struct SetVelocityCommand {
    EntityId target;
    Vec2 velocity_km_per_second;
};

using CommandPayload = std::variant<SetVelocityCommand>;

struct Command {
    Tick execute_on{0};
    CommandPayload payload;
};

inline Command set_velocity_at(Tick execute_on, EntityId target, Vec2 velocity_km_per_second) {
    return Command{execute_on, SetVelocityCommand{target, velocity_km_per_second}};
}

} // namespace scs::domain
