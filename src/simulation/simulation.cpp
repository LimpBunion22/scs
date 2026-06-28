#include "simulation/simulation.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace scs::simulation {
namespace {

domain::EntityId command_subject(const domain::Command& command) {
    return std::visit(
        [](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, domain::SetVelocityCommand>) {
                return payload.target;
            } else {
                return payload.launcher;
            }
        },
        command.payload);
}

std::string entity_message(const char* text, domain::EntityId id) {
    std::ostringstream stream;
    stream << text << " " << id.value;
    return stream.str();
}

std::string engagement_message(const char* text,
                               domain::EntityId launcher,
                               domain::EntityId target) {
    std::ostringstream stream;
    stream << text << " launcher " << launcher.value << " target " << target.value;
    return stream.str();
}

std::string contact_message(const char* text, domain::ContactId id, domain::EntityId observer) {
    std::ostringstream stream;
    stream << text << " " << id.value << " observed by entity " << observer.value;
    return stream.str();
}

std::string missile_message(const char* text,
                            domain::MissileId id,
                            domain::EntityId launcher,
                            domain::EntityId target) {
    std::ostringstream stream;
    stream << text << " " << id.value << " launcher " << launcher.value << " target " << target.value;
    return stream.str();
}

bool is_opposed(domain::Allegiance observer, domain::Allegiance target) {
    return (observer == domain::Allegiance::Friendly && target == domain::Allegiance::Hostile) ||
           (observer == domain::Allegiance::Hostile && target == domain::Allegiance::Friendly);
}

} // namespace

Simulation::Simulation(Scenario scenario)
    : scenario_name_(std::move(scenario.name)),
      seed_(scenario.seed),
      fixed_step_seconds_(scenario.fixed_step_seconds),
      entities_(std::move(scenario.entities)) {
    validate_initial_state();

    std::sort(entities_.begin(), entities_.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.id < rhs.id;
    });

    append_event(domain::EventSeverity::Info,
                 domain::EventType::ScenarioLoaded,
                 domain::EntityId{},
                 "Scenario loaded: " + scenario_name_);
    update_contacts();
}

bool Simulation::submit(domain::Command command) {
    const domain::EntityId subject = command_subject(command);
    if (command.execute_on < current_tick_) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     subject,
                     entity_message("Rejected command scheduled in the past for entity", subject));
        return false;
    }

    if (find_entity(subject) == nullptr) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     subject,
                     entity_message("Rejected command for unknown entity", subject));
        return false;
    }

    command_queue_.push_back(QueuedCommand{std::move(command), next_command_sequence_++});
    std::stable_sort(command_queue_.begin(), command_queue_.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.command.execute_on != rhs.command.execute_on) {
            return lhs.command.execute_on < rhs.command.execute_on;
        }
        return lhs.sequence < rhs.sequence;
    });

    append_event(domain::EventSeverity::Info,
                 domain::EventType::CommandAccepted,
                 subject,
                 entity_message("Accepted command for entity", subject));
    return true;
}

void Simulation::advance(domain::Tick ticks) {
    for (domain::Tick i = 0; i < ticks; ++i) {
        advance_one_tick();
    }
}

void Simulation::advance_one_tick() {
    apply_due_commands();

    for (auto& entity : entities_) {
        entity.position_km += entity.velocity_km_per_second * fixed_step_seconds_;
    }

    ++current_tick_;
    update_missiles();
    update_contacts();
}

domain::WorldSnapshot Simulation::snapshot() const {
    domain::WorldSnapshot result;
    result.tick = current_tick_;
    result.time_seconds = time_seconds();
    result.entities.reserve(entities_.size());

    for (const auto& entity : entities_) {
        result.entities.push_back(domain::EntitySnapshot{
            entity.id,
            entity.kind,
            entity.allegiance,
            entity.name,
            entity.position_km,
            entity.velocity_km_per_second,
            entity.missile_ammunition,
            entity.defensive_response_charges,
        });
    }
    result.contacts = contact_tracker_.snapshots();
    result.missiles.reserve(missiles_.size());
    for (const auto& missile : missiles_) {
        result.missiles.push_back(make_missile_snapshot(missile));
    }

    return result;
}

const std::vector<domain::Event>& Simulation::events() const noexcept {
    return events_;
}

domain::Tick Simulation::current_tick() const noexcept {
    return current_tick_;
}

double Simulation::time_seconds() const noexcept {
    return static_cast<double>(current_tick_) * fixed_step_seconds_;
}

double Simulation::fixed_step_seconds() const noexcept {
    return fixed_step_seconds_;
}

std::uint64_t Simulation::seed() const noexcept {
    return seed_;
}

domain::EntityState* Simulation::find_entity(domain::EntityId id) {
    auto found = std::find_if(entities_.begin(), entities_.end(), [id](const auto& entity) {
        return entity.id == id;
    });
    return found == entities_.end() ? nullptr : &(*found);
}

const domain::EntityState* Simulation::find_entity(domain::EntityId id) const {
    auto found = std::find_if(entities_.begin(), entities_.end(), [id](const auto& entity) {
        return entity.id == id;
    });
    return found == entities_.end() ? nullptr : &(*found);
}

void Simulation::validate_initial_state() const {
    if (fixed_step_seconds_ <= 0.0) {
        throw std::invalid_argument("Simulation fixed step must be positive.");
    }

    std::vector<domain::EntityId> ids;
    ids.reserve(entities_.size());
    for (const auto& entity : entities_) {
        if (!domain::is_valid(entity.id)) {
            throw std::invalid_argument("Scenario contains an invalid entity id.");
        }
        ids.push_back(entity.id);
        if (entity.missile_ammunition < 0) {
            throw std::invalid_argument("Scenario contains negative missile ammunition.");
        }
        if (entity.defensive_response_charges < 0) {
            throw std::invalid_argument("Scenario contains negative defensive response charges.");
        }
    }

    std::sort(ids.begin(), ids.end());
    const auto duplicate = std::adjacent_find(ids.begin(), ids.end());
    if (duplicate != ids.end()) {
        throw std::invalid_argument("Scenario contains duplicate entity ids.");
    }
}

void Simulation::apply_due_commands() {
    auto first_pending = command_queue_.begin();
    while (first_pending != command_queue_.end() && first_pending->command.execute_on <= current_tick_) {
        apply_command(first_pending->command);
        ++first_pending;
    }
    command_queue_.erase(command_queue_.begin(), first_pending);
}

void Simulation::apply_command(const domain::Command& command) {
    std::visit(
        [this](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, domain::SetVelocityCommand>) {
                auto* entity = find_entity(payload.target);
                if (entity == nullptr) {
                    append_event(domain::EventSeverity::Advisory,
                                 domain::EventType::CommandRejected,
                                 payload.target,
                                 entity_message("Skipped command for missing entity", payload.target));
                    return;
                }

                entity->velocity_km_per_second = payload.velocity_km_per_second;
                append_event(domain::EventSeverity::Info,
                             domain::EventType::VelocityChanged,
                             payload.target,
                             entity_message("Velocity updated for entity", payload.target));
            } else if constexpr (std::is_same_v<Payload, domain::EngageEntityCommand>) {
                apply_engage_entity_command(payload);
            } else if constexpr (std::is_same_v<Payload, domain::EngageContactCommand>) {
                apply_engage_contact_command(payload);
            }
        },
        command.payload);
}

void Simulation::apply_engage_entity_command(const domain::EngageEntityCommand& command) {
    auto* launcher = find_entity(command.launcher);
    if (launcher == nullptr) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     command.launcher,
                     entity_message("Skipped engagement for missing launcher", command.launcher));
        return;
    }

    const auto* target = find_entity(command.target);
    if (target == nullptr) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     command.launcher,
                     engagement_message("Rejected engagement for missing target", command.launcher, command.target));
        return;
    }

    try_launch_missile(*launcher, *target, domain::ContactId{});
}

void Simulation::apply_engage_contact_command(const domain::EngageContactCommand& command) {
    auto* launcher = find_entity(command.launcher);
    if (launcher == nullptr) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     command.launcher,
                     entity_message("Skipped engagement for missing launcher", command.launcher));
        return;
    }

    const auto contacts = contact_tracker_.snapshots();
    const auto found = std::find_if(contacts.begin(), contacts.end(), [command](const auto& contact) {
        return contact.id == command.target;
    });
    if (found == contacts.end() || found->observer != command.launcher) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     command.launcher,
                     contact_message("Rejected engagement for unavailable contact", command.target, command.launcher));
        return;
    }

    auto* target = resolve_contact_target(*launcher, *found);
    if (target == nullptr) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     command.launcher,
                     contact_message("Rejected engagement for unresolved contact", command.target, command.launcher));
        return;
    }

    try_launch_missile(*launcher, *target, command.target);
}

void Simulation::try_launch_missile(domain::EntityState& launcher,
                                    const domain::EntityState& target,
                                    domain::ContactId target_contact) {
    if (launcher.id == target.id || !is_opposed(launcher.allegiance, target.allegiance)) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     launcher.id,
                     engagement_message("Rejected engagement against invalid target", launcher.id, target.id));
        return;
    }

    if (launcher.missile_ammunition <= 0) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     launcher.id,
                     entity_message("Rejected engagement with no missile ammunition for entity", launcher.id));
        return;
    }

    --launcher.missile_ammunition;
    const auto missile_id = next_missile_id_;
    next_missile_id_ = domain::MissileId{next_missile_id_.value + 1};

    missiles_.push_back(MissileState{
        missile_id,
        launcher.id,
        target.id,
        target_contact,
        launcher.position_km,
        domain::Vec2{},
        domain::MissileStatus::InFlight,
        false,
    });

    append_event(domain::EventSeverity::Info,
                 domain::EventType::MissileLaunched,
                 launcher.id,
                 missile_message("Launched missile", missile_id, launcher.id, target.id));
}

domain::EntityState* Simulation::resolve_contact_target(const domain::EntityState& launcher,
                                                        const domain::ContactSnapshot& contact) {
    domain::EntityState* closest = nullptr;
    double closest_distance_squared = std::numeric_limits<double>::infinity();

    // ponytail: Contact snapshots intentionally hide target IDs. The two-group
    // slice resolves contact engagements to the nearest opposed entity; replace
    // this with an internal track lookup before supporting ambiguous contacts.
    for (auto& entity : entities_) {
        if (entity.id == launcher.id || !is_opposed(launcher.allegiance, entity.allegiance)) {
            continue;
        }

        const auto offset = entity.position_km - contact.estimated_position_km;
        const double distance_squared = domain::magnitude_squared(offset);
        if (distance_squared < closest_distance_squared) {
            closest = &entity;
            closest_distance_squared = distance_squared;
        }
    }

    return closest;
}

void Simulation::update_missiles() {
    for (auto& missile : missiles_) {
        if (missile.status != domain::MissileStatus::InFlight) {
            continue;
        }

        auto* target = find_entity(missile.target_entity);
        if (target == nullptr) {
            missile.status = domain::MissileStatus::Miss;
            append_event(domain::EventSeverity::Advisory,
                         domain::EventType::MissileMissed,
                         missile.launcher,
                         missile_message("Missile lost target", missile.id, missile.launcher, missile.target_entity));
            continue;
        }

        const auto offset = target->position_km - missile.position_km;
        const double distance = std::sqrt(domain::magnitude_squared(offset));
        if (distance > 0.0) {
            const auto direction = offset * (1.0 / distance);
            const double step_distance = default_missile_constants.speed_km_per_second * fixed_step_seconds_;
            missile.velocity_km_per_second = direction * default_missile_constants.speed_km_per_second;
            if (step_distance >= distance) {
                missile.position_km = target->position_km;
            } else {
                missile.position_km += direction * step_distance;
            }
        } else {
            missile.velocity_km_per_second = domain::Vec2{};
        }

        const auto remaining_offset = target->position_km - missile.position_km;
        const double remaining_distance = std::sqrt(domain::magnitude_squared(remaining_offset));

        if (!missile.threat_event_emitted &&
            remaining_distance <= default_missile_constants.threat_range_km) {
            missile.threat_event_emitted = true;
            append_event(domain::EventSeverity::Threat,
                         domain::EventType::MissileThreat,
                         target->id,
                         missile_message("Missile entering threat range", missile.id, missile.launcher, target->id));
        }

        if (target->defensive_response_charges > 0 &&
            remaining_distance <= default_missile_constants.defense_range_km) {
            --target->defensive_response_charges;
            missile.status = domain::MissileStatus::Defeated;
            append_event(domain::EventSeverity::Critical,
                         domain::EventType::DefensiveResponse,
                         target->id,
                         missile_message("Defensive response defeated missile", missile.id, missile.launcher, target->id));
            append_event(domain::EventSeverity::Advisory,
                         domain::EventType::MissileMissed,
                         target->id,
                         missile_message("Missile missed after defensive response", missile.id, missile.launcher, target->id));
            continue;
        }

        if (remaining_distance <= default_missile_constants.hit_radius_km) {
            missile.status = domain::MissileStatus::Hit;
            append_event(domain::EventSeverity::Critical,
                         domain::EventType::MissileHit,
                         target->id,
                         missile_message("Missile hit target", missile.id, missile.launcher, target->id));
        }
    }
}

void Simulation::update_contacts() {
    for (const auto& contact_event : contact_tracker_.update(current_tick_, fixed_step_seconds_, entities_)) {
        append_event(contact_event.severity,
                     contact_event.type,
                     contact_event.observer,
                     contact_message(contact_event.type == domain::EventType::ContactDetected ? "Detected contact"
                                                                                               : "Updated contact",
                                     contact_event.contact,
                                     contact_event.observer));
    }
}

void Simulation::append_event(domain::EventSeverity severity,
                              domain::EventType type,
                              domain::EntityId subject,
                              std::string message) {
    events_.push_back(domain::Event{current_tick_, severity, type, subject, std::move(message)});
}

} // namespace scs::simulation
