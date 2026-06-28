#pragma once

#include <variant>

#include "domain/contact.h"
#include "domain/ids.h"
#include "domain/time.h"
#include "domain/vector2.h"

namespace scs::domain {

struct SetVelocityCommand {
    EntityId target;
    Vec2 velocity_km_per_second;
};

struct EngageEntityCommand {
    EntityId launcher;
    EntityId target;
};

struct EngageContactCommand {
    EntityId launcher;
    ContactId target;
};

using CommandPayload = std::variant<SetVelocityCommand, EngageEntityCommand, EngageContactCommand>;

struct Command {
    Tick execute_on{0};
    CommandPayload payload;
};

inline Command set_velocity_at(Tick execute_on, EntityId target, Vec2 velocity_km_per_second) {
    return Command{execute_on, SetVelocityCommand{target, velocity_km_per_second}};
}

inline Command engage_entity_at(Tick execute_on, EntityId launcher, EntityId target) {
    return Command{execute_on, EngageEntityCommand{launcher, target}};
}

inline Command engage_contact_at(Tick execute_on, EntityId launcher, ContactId target) {
    return Command{execute_on, EngageContactCommand{launcher, target}};
}

} // namespace scs::domain
