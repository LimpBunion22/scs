#include "simulation/simulation.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace scs::simulation {
namespace {

domain::EntityId command_target(const domain::Command& command) {
    return std::visit([](const auto& payload) { return payload.target; }, command.payload);
}

std::string entity_message(const char* text, domain::EntityId id) {
    std::ostringstream stream;
    stream << text << " " << id.value;
    return stream.str();
}

std::string contact_message(const char* text, domain::ContactId id, domain::EntityId observer) {
    std::ostringstream stream;
    stream << text << " " << id.value << " observed by entity " << observer.value;
    return stream.str();
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
    const domain::EntityId target = command_target(command);
    if (command.execute_on < current_tick_) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     target,
                     entity_message("Rejected command scheduled in the past for entity", target));
        return false;
    }

    if (find_entity(target) == nullptr) {
        append_event(domain::EventSeverity::Advisory,
                     domain::EventType::CommandRejected,
                     target,
                     entity_message("Rejected command for unknown entity", target));
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
                 target,
                 entity_message("Accepted command for entity", target));
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
        });
    }
    result.contacts = contact_tracker_.snapshots();

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
            }
        },
        command.payload);
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
