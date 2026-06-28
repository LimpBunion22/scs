#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "domain/command.h"
#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_projection.h"
#include "simulation/replay.h"
#include "simulation/scenario.h"
#include "simulation/simulation.h"
#include "ui/desktop_interaction.h"
#include "ui/desktop_order_model.h"

namespace {

[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error(message);
}

void require(bool condition, const std::string& message) {
    if (!condition) {
        fail(message);
    }
}

bool close(double lhs, double rhs) {
    return std::abs(lhs - rhs) < 1e-9;
}

std::string path_for(const char* collection, std::size_t index, const char* field) {
    std::ostringstream stream;
    stream << collection << '[' << index << "]." << field;
    return stream.str();
}

void require_same_double(double lhs, double rhs, const std::string& path) {
    if (!close(lhs, rhs)) {
        fail("Snapshot divergence at " + path + ".");
    }
}

void require_same_vec(scs::domain::Vec2 lhs, scs::domain::Vec2 rhs, const std::string& path) {
    require_same_double(lhs.x, rhs.x, path + ".x");
    require_same_double(lhs.y, rhs.y, path + ".y");
}

void require_same_contacts(const std::vector<scs::domain::ContactSnapshot>& lhs,
                           const std::vector<scs::domain::ContactSnapshot>& rhs) {
    if (lhs.size() != rhs.size()) {
        fail("Snapshot divergence at contacts.size.");
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i].id != rhs[i].id) {
            fail("Snapshot divergence at " + path_for("contacts", i, "id") + ".");
        }
        if (lhs[i].observer != rhs[i].observer) {
            fail("Snapshot divergence at " + path_for("contacts", i, "observer") + ".");
        }
        require_same_vec(lhs[i].estimated_position_km,
                         rhs[i].estimated_position_km,
                         path_for("contacts", i, "estimated_position_km"));
        require_same_vec(lhs[i].estimated_velocity_km_per_second,
                         rhs[i].estimated_velocity_km_per_second,
                         path_for("contacts", i, "estimated_velocity_km_per_second"));
        if (lhs[i].last_observed_tick != rhs[i].last_observed_tick) {
            fail("Snapshot divergence at " + path_for("contacts", i, "last_observed_tick") + ".");
        }
        require_same_double(lhs[i].confidence, rhs[i].confidence, path_for("contacts", i, "confidence"));
        if (lhs[i].classification != rhs[i].classification) {
            fail("Snapshot divergence at " + path_for("contacts", i, "classification") + ".");
        }
        require_same_double(lhs[i].uncertainty_radius_km,
                            rhs[i].uncertainty_radius_km,
                            path_for("contacts", i, "uncertainty_radius_km"));
    }
}

void require_same_missiles(const std::vector<scs::domain::MissileSnapshot>& lhs,
                           const std::vector<scs::domain::MissileSnapshot>& rhs) {
    if (lhs.size() != rhs.size()) {
        fail("Snapshot divergence at missiles.size.");
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i].id != rhs[i].id) {
            fail("Snapshot divergence at " + path_for("missiles", i, "id") + ".");
        }
        if (lhs[i].launcher != rhs[i].launcher) {
            fail("Snapshot divergence at " + path_for("missiles", i, "launcher") + ".");
        }
        if (lhs[i].target_entity != rhs[i].target_entity) {
            fail("Snapshot divergence at " + path_for("missiles", i, "target_entity") + ".");
        }
        if (lhs[i].target_contact != rhs[i].target_contact) {
            fail("Snapshot divergence at " + path_for("missiles", i, "target_contact") + ".");
        }
        require_same_vec(lhs[i].position_km, rhs[i].position_km, path_for("missiles", i, "position_km"));
        require_same_vec(lhs[i].velocity_km_per_second,
                         rhs[i].velocity_km_per_second,
                         path_for("missiles", i, "velocity_km_per_second"));
        if (lhs[i].status != rhs[i].status) {
            fail("Snapshot divergence at " + path_for("missiles", i, "status") + ".");
        }
    }
}

void require_same_snapshot(const scs::domain::WorldSnapshot& lhs,
                           const scs::domain::WorldSnapshot& rhs) {
    if (lhs.tick != rhs.tick) {
        fail("Snapshot divergence at tick.");
    }
    require_same_double(lhs.time_seconds, rhs.time_seconds, "time_seconds");

    if (lhs.entities.size() != rhs.entities.size()) {
        fail("Snapshot divergence at entities.size.");
    }

    for (std::size_t i = 0; i < lhs.entities.size(); ++i) {
        const auto& left = lhs.entities[i];
        const auto& right = rhs.entities[i];
        if (left.id != right.id) {
            fail("Snapshot divergence at " + path_for("entities", i, "id") + ".");
        }
        if (left.kind != right.kind) {
            fail("Snapshot divergence at " + path_for("entities", i, "kind") + ".");
        }
        if (left.allegiance != right.allegiance) {
            fail("Snapshot divergence at " + path_for("entities", i, "allegiance") + ".");
        }
        if (left.name != right.name) {
            fail("Snapshot divergence at " + path_for("entities", i, "name") + ".");
        }
        require_same_vec(left.position_km, right.position_km, path_for("entities", i, "position_km"));
        require_same_vec(left.velocity_km_per_second,
                         right.velocity_km_per_second,
                         path_for("entities", i, "velocity_km_per_second"));
        if (left.missile_ammunition != right.missile_ammunition) {
            fail("Snapshot divergence at " + path_for("entities", i, "missile_ammunition") + ".");
        }
        if (left.defensive_response_charges != right.defensive_response_charges) {
            fail("Snapshot divergence at " + path_for("entities", i, "defensive_response_charges") + ".");
        }
    }

    require_same_contacts(lhs.contacts, rhs.contacts);
    require_same_missiles(lhs.missiles, rhs.missiles);
}

void require_same_events(const std::vector<scs::domain::Event>& lhs,
                         const std::vector<scs::domain::Event>& rhs) {
    if (lhs.size() != rhs.size()) {
        fail("Event divergence at events.size.");
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i].tick != rhs[i].tick) {
            fail("Event divergence at " + path_for("events", i, "tick") + ".");
        }
        if (lhs[i].severity != rhs[i].severity) {
            fail("Event divergence at " + path_for("events", i, "severity") + ".");
        }
        if (lhs[i].type != rhs[i].type) {
            fail("Event divergence at " + path_for("events", i, "type") + ".");
        }
        if (lhs[i].subject != rhs[i].subject) {
            fail("Event divergence at " + path_for("events", i, "subject") + ".");
        }
        if (lhs[i].message != rhs[i].message) {
            fail("Event divergence at " + path_for("events", i, "message") + ".");
        }
    }
}

const scs::domain::EntitySnapshot& friendly_by_id(
    const scs::presentation::TacticalSnapshot& snapshot,
    scs::domain::EntityId id) {
    for (const auto& entity : snapshot.friendly_entities) {
        if (entity.id == id) {
            return entity;
        }
    }
    fail("Missing expected friendly in tactical snapshot.");
}

const scs::domain::ContactSnapshot& contact_by_id(
    const scs::presentation::TacticalSnapshot& snapshot,
    scs::domain::ContactId id) {
    for (const auto& contact : snapshot.hostile_contacts) {
        if (contact.id == id) {
            return contact;
        }
    }
    fail("Missing expected contact in tactical snapshot.");
}

const scs::domain::EntitySnapshot& entity_by_id(const scs::domain::WorldSnapshot& snapshot,
                                                scs::domain::EntityId id) {
    for (const auto& entity : snapshot.entities) {
        if (entity.id == id) {
            return entity;
        }
    }
    fail("Missing expected entity in world snapshot.");
}

const scs::domain::MissileSnapshot& only_missile(const scs::domain::WorldSnapshot& snapshot) {
    require(snapshot.missiles.size() == 1, "Expected exactly one missile in final snapshot.");
    return snapshot.missiles.front();
}

bool has_event(const std::vector<scs::domain::Event>& events,
               scs::domain::Tick tick,
               scs::domain::EventType type,
               scs::domain::EventSeverity severity,
               scs::domain::EntityId subject) {
    for (const auto& event : events) {
        if (event.tick == tick &&
            event.type == type &&
            event.severity == severity &&
            event.subject == subject) {
            return true;
        }
    }
    return false;
}

scs::rendering::TacticalMapProjection make_projection() {
    return scs::rendering::TacticalMapProjection{
        scs::domain::Vec2{175.0, 0.0},
        1.0,
        scs::rendering::ScreenRect{0.0, 0.0, 800.0, 600.0},
    };
}

struct DesktopFlowResult {
    scs::domain::WorldSnapshot final_snapshot;
    std::vector<scs::domain::Event> events;
    std::vector<scs::domain::Command> commands;
    scs::domain::Tick ticks_run{0};
};

DesktopFlowResult run_scripted_desktop_command_flow() {
    scs::simulation::Simulation simulation(
        scs::simulation::make_playable_engagement_demo_scenario());
    const auto tactical = scs::presentation::make_tactical_snapshot(
        simulation.snapshot(),
        simulation.events(),
        scs::presentation::TacticalSnapshotOptions{});

    require(tactical.tick == 0, "Desktop flow should start from tick 0.");
    require(tactical.friendly_entities.size() == 1, "Desktop flow expected one visible friendly.");
    require(tactical.hostile_contacts.size() == 1, "Desktop flow expected one hostile contact.");

    const auto& launcher = friendly_by_id(tactical, scs::domain::EntityId{1});
    const auto& contact = contact_by_id(tactical, scs::domain::ContactId{1});
    const auto projection = make_projection();

    scs::ui::DesktopInteractionState state;
    scs::ui::select_desktop_map_object(
        state,
        tactical,
        projection,
        scs::rendering::world_to_screen(launcher.position_km, projection));

    require(state.staged_launcher == launcher.id, "Desktop selection did not stage the launcher.");
    state.staged_velocity_x_km_per_second = 1.0;
    state.staged_velocity_y_km_per_second = 0.0;

    std::vector<scs::domain::Command> commands;
    const auto velocity = scs::ui::emit_desktop_velocity_command(state, tactical);
    require(velocity.command.has_value(), "Desktop velocity helper did not emit a command.");
    require(simulation.submit(*velocity.command), "Desktop velocity command was not accepted.");
    commands.push_back(*velocity.command);

    scs::ui::select_desktop_map_object(
        state,
        tactical,
        projection,
        scs::rendering::world_to_screen(contact.estimated_position_km, projection));

    require(state.staged_target == contact.id, "Desktop selection did not stage the target.");
    const auto engagement = scs::ui::emit_staged_desktop_engage_contact(state, tactical);
    require(engagement.command.has_value(), "Desktop engage helper did not emit a command.");
    require(simulation.submit(*engagement.command), "Desktop engage command was not accepted.");
    commands.push_back(*engagement.command);

    constexpr scs::domain::Tick ticks_to_run = 4;
    simulation.advance(ticks_to_run);

    return DesktopFlowResult{
        simulation.snapshot(),
        simulation.events(),
        commands,
        ticks_to_run,
    };
}

void desktop_flow_submits_commands_and_resolves_engagement() {
    const auto result = run_scripted_desktop_command_flow();

    require(result.commands.size() == 2, "Desktop flow command count changed.");
    require(std::holds_alternative<scs::domain::SetVelocityCommand>(result.commands[0].payload),
            "Desktop flow did not emit maneuver command first.");
    require(std::holds_alternative<scs::domain::EngageContactCommand>(result.commands[1].payload),
            "Desktop flow did not emit engage-contact command second.");

    const auto& blue = entity_by_id(result.final_snapshot, scs::domain::EntityId{1});
    const auto& red = entity_by_id(result.final_snapshot, scs::domain::EntityId{2});
    const auto& missile = only_missile(result.final_snapshot);

    require(result.final_snapshot.tick == 4, "Desktop flow final tick changed.");
    require(close(result.final_snapshot.time_seconds, 4.0), "Desktop flow final time changed.");
    require(blue.missile_ammunition == 0, "Desktop flow did not consume launcher ammunition.");
    require(close(blue.velocity_km_per_second.x, 1.0), "Desktop maneuver velocity x was not applied.");
    require(close(blue.position_km.x, 4.0), "Desktop maneuver did not advance from explicit ticks.");
    require(red.defensive_response_charges == 0, "Desktop flow did not consume target defense.");
    require(missile.target_contact == scs::domain::ContactId{1},
            "Desktop flow missile did not preserve contact target.");
    require(missile.status == scs::domain::MissileStatus::Defeated,
            "Desktop flow missile should be defeated by defensive response.");

    require(has_event(result.events,
                      0,
                      scs::domain::EventType::VelocityChanged,
                      scs::domain::EventSeverity::Info,
                      scs::domain::EntityId{1}),
            "Desktop flow did not record maneuver execution.");
    require(has_event(result.events,
                      0,
                      scs::domain::EventType::MissileLaunched,
                      scs::domain::EventSeverity::Info,
                      scs::domain::EntityId{1}),
            "Desktop flow did not record missile launch.");
    require(has_event(result.events,
                      1,
                      scs::domain::EventType::MissileThreat,
                      scs::domain::EventSeverity::Threat,
                      scs::domain::EntityId{2}),
            "Desktop flow did not record missile threat.");
    require(has_event(result.events,
                      3,
                      scs::domain::EventType::DefensiveResponse,
                      scs::domain::EventSeverity::Critical,
                      scs::domain::EntityId{2}),
            "Desktop flow did not record defensive response.");
    require(has_event(result.events,
                      3,
                      scs::domain::EventType::MissileMissed,
                      scs::domain::EventSeverity::Advisory,
                      scs::domain::EntityId{2}),
            "Desktop flow did not record defended missile miss.");
}

void desktop_flow_replays_to_identical_snapshot_and_events() {
    const auto desktop = run_scripted_desktop_command_flow();
    const auto replay = scs::simulation::run_replay(scs::simulation::ReplayInput{
        scs::simulation::make_playable_engagement_demo_scenario(),
        desktop.commands,
        desktop.ticks_run,
    });

    require_same_snapshot(desktop.final_snapshot, replay.final_snapshot);
    require_same_events(desktop.events, replay.events);
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"desktop_flow_submits_commands_and_resolves_engagement",
         desktop_flow_submits_commands_and_resolves_engagement},
        {"desktop_flow_replays_to_identical_snapshot_and_events",
         desktop_flow_replays_to_identical_snapshot_and_events},
    };

    for (const auto& test : tests) {
        try {
            test.second();
            std::cout << "[PASS] " << test.first << '\n';
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << test.first << ": " << error.what() << '\n';
            return 1;
        }
    }

    return 0;
}
