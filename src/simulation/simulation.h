#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "domain/command.h"
#include "domain/entity.h"
#include "domain/event.h"
#include "domain/snapshot.h"
#include "domain/time.h"
#include "simulation/scenario.h"

namespace scs::simulation {

class Simulation {
public:
    explicit Simulation(Scenario scenario);

    bool submit(domain::Command command);
    void advance(domain::Tick ticks);
    void advance_one_tick();

    [[nodiscard]] domain::WorldSnapshot snapshot() const;
    [[nodiscard]] const std::vector<domain::Event>& events() const noexcept;
    [[nodiscard]] domain::Tick current_tick() const noexcept;
    [[nodiscard]] double time_seconds() const noexcept;
    [[nodiscard]] double fixed_step_seconds() const noexcept;
    [[nodiscard]] std::uint64_t seed() const noexcept;

private:
    struct QueuedCommand {
        domain::Command command;
        std::uint64_t sequence{0};
    };

    std::string scenario_name_;
    std::uint64_t seed_{0};
    double fixed_step_seconds_{1.0};
    domain::Tick current_tick_{0};
    std::uint64_t next_command_sequence_{0};
    std::vector<domain::EntityState> entities_;
    std::vector<QueuedCommand> command_queue_;
    std::vector<domain::Event> events_;

    [[nodiscard]] domain::EntityState* find_entity(domain::EntityId id);
    [[nodiscard]] const domain::EntityState* find_entity(domain::EntityId id) const;

    void validate_initial_state() const;
    void apply_due_commands();
    void apply_command(const domain::Command& command);
    void append_event(domain::EventSeverity severity,
                      domain::EventType type,
                      domain::EntityId subject,
                      std::string message);
};

} // namespace scs::simulation
