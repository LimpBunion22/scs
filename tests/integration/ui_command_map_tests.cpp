#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_renderer.h"
#include "simulation/scenario.h"
#include "simulation/simulation.h"
#include "ui/tactical_command_ui.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool close(double lhs, double rhs) {
    return std::abs(lhs - rhs) < 1e-9;
}

const scs::domain::EntitySnapshot& entity_by_id(const scs::domain::WorldSnapshot& snapshot,
                                                scs::domain::EntityId id) {
    for (const auto& entity : snapshot.entities) {
        if (entity.id == id) {
            return entity;
        }
    }
    throw std::runtime_error("Missing entity in snapshot.");
}

scs::presentation::TacticalSnapshot make_tactical_fixture() {
    scs::presentation::TacticalSnapshot snapshot;
    snapshot.tick = 42;
    snapshot.time_seconds = 42.0;
    snapshot.friendly_entities.push_back(scs::domain::EntitySnapshot{
        scs::domain::EntityId{1},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Friendly,
        "Blue Fixture",
        scs::domain::Vec2{0.0, 0.0},
        scs::domain::Vec2{10.0, 0.0},
        2,
        1,
    });
    snapshot.hostile_contacts.push_back(scs::domain::ContactSnapshot{
        scs::domain::ContactId{7},
        scs::domain::EntityId{1},
        scs::domain::Vec2{200.0, 100.0},
        scs::domain::Vec2{-1.0, 0.0},
        40,
        0.75,
        scs::domain::ContactClassification::HostileCombatGroup,
        25.0,
    });
    snapshot.events.push_back(scs::domain::Event{
        42,
        scs::domain::EventSeverity::Advisory,
        scs::domain::EventType::ContactDetected,
        scs::domain::EntityId{1},
        "Contact detected for test.",
    });
    snapshot.predicted_trajectories.push_back(scs::presentation::TacticalTrajectory{
        scs::presentation::TacticalTrajectorySourceKind::FriendlyEntity,
        scs::domain::EntityId{1},
        scs::domain::ContactId{},
        {
            scs::presentation::TrajectoryPoint{42, 42.0, scs::domain::Vec2{0.0, 0.0}},
            scs::presentation::TrajectoryPoint{43, 43.0, scs::domain::Vec2{10.0, 0.0}},
        },
    });
    snapshot.predicted_trajectories.push_back(scs::presentation::TacticalTrajectory{
        scs::presentation::TacticalTrajectorySourceKind::HostileContact,
        scs::domain::EntityId{},
        scs::domain::ContactId{7},
        {
            scs::presentation::TrajectoryPoint{42, 42.0, scs::domain::Vec2{200.0, 100.0}},
            scs::presentation::TrajectoryPoint{43, 43.0, scs::domain::Vec2{199.0, 100.0}},
        },
    });
    return snapshot;
}

void renderer_shows_map_contacts_selection_and_events() {
    const auto snapshot = make_tactical_fixture();
    const auto output = scs::rendering::render_tactical_map(
        snapshot,
        scs::rendering::TacticalMapView{scs::domain::Vec2{0.0, 0.0}, 100.0, 21, 11},
        scs::rendering::TacticalSelection{
            scs::rendering::TacticalSelectionKind::HostileContact,
            scs::domain::EntityId{},
            scs::domain::ContactId{7},
        });

    require(output.find("Tactical map tick=42") != std::string::npos,
            "Renderer did not include simulation tick.");
    require(output.find("F1 Blue Fixture") != std::string::npos,
            "Renderer did not list the friendly group.");
    require(output.find("C7 observer=F1") != std::string::npos,
            "Renderer did not list the hostile contact.");
    require(output.find("contact_detected") != std::string::npos,
            "Renderer did not include event log entries.");
    require(output.find('*') != std::string::npos,
            "Renderer did not mark the selected map object.");
}

void ui_controls_view_pause_time_scale_and_velocity_command() {
    const auto snapshot = make_tactical_fixture();
    scs::ui::TacticalUiState state;
    state.view.kilometers_per_cell = 100.0;

    auto result = scs::ui::handle_tactical_input(state, snapshot, "pan 2 -1");
    require(!result.command.has_value(), "Pan should not emit a simulation command.");
    require(close(state.view.center_km.x, 200.0), "Pan did not move east by view cells.");
    require(close(state.view.center_km.y, -100.0), "Pan did not move north by view cells.");

    result = scs::ui::handle_tactical_input(state, snapshot, "zoom in");
    require(!result.command.has_value(), "Zoom should not emit a simulation command.");
    require(close(state.view.kilometers_per_cell, 50.0), "Zoom in did not reduce map scale.");

    result = scs::ui::handle_tactical_input(state, snapshot, "pause");
    require(!result.command.has_value(), "Pause should not emit a simulation command.");
    require(scs::ui::make_time_scale_input(state).tactical_pause,
            "Pause did not update player time-scale input.");
    result = scs::ui::handle_tactical_input(state, snapshot, "run 5");
    require(result.advance_ticks == 0, "Run should not advance while paused.");

    result = scs::ui::handle_tactical_input(state, snapshot, "resume");
    require(!result.command.has_value(), "Resume should not emit a simulation command.");
    result = scs::ui::handle_tactical_input(state, snapshot, "run 5");
    require(result.advance_ticks == 5, "Run did not request explicit tick advancement.");

    result = scs::ui::handle_tactical_input(state, snapshot, "select friendly 1");
    require(!result.command.has_value(), "Selection should not emit a simulation command.");
    result = scs::ui::handle_tactical_input(state, snapshot, "velocity 12.5 -0.5");

    require(result.command.has_value(), "Velocity command was not emitted.");
    require(result.command->execute_on == snapshot.tick, "Velocity command used the wrong tick.");
    const auto* payload = std::get_if<scs::domain::SetVelocityCommand>(&result.command->payload);
    require(payload != nullptr, "Velocity command emitted the wrong command payload.");
    require(payload->target == scs::domain::EntityId{1}, "Velocity command targeted the wrong entity.");
    require(close(payload->velocity_km_per_second.x, 12.5), "Velocity command x component changed.");
    require(close(payload->velocity_km_per_second.y, -0.5), "Velocity command y component changed.");
}

void ui_command_emission_changes_simulation_only_after_submission_and_tick() {
    scs::simulation::Simulation simulation(scs::simulation::make_default_vertical_slice_scenario());
    scs::ui::TacticalUiState state;
    state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::FriendlyEntity,
        scs::domain::EntityId{1},
        scs::domain::ContactId{},
    };
    const auto tactical = scs::presentation::make_tactical_snapshot(
        simulation.snapshot(),
        simulation.events(),
        scs::presentation::TacticalSnapshotOptions{scs::presentation::PredictionConfig{1}});

    auto result = scs::ui::handle_tactical_input(state, tactical, "velocity 21 0.25");
    require(result.command.has_value(), "UI did not emit a velocity command.");

    const auto before_submit = simulation.snapshot();
    require(simulation.submit(*result.command), "Simulation rejected emitted UI command.");
    const auto after_submit = simulation.snapshot();
    require(close(entity_by_id(before_submit, scs::domain::EntityId{1}).velocity_km_per_second.x,
                  entity_by_id(after_submit, scs::domain::EntityId{1}).velocity_km_per_second.x),
            "Submitting the UI command directly mutated entity velocity.");

    simulation.advance_one_tick();
    const auto after_tick = simulation.snapshot();
    require(close(entity_by_id(after_tick, scs::domain::EntityId{1}).velocity_km_per_second.x, 21.0),
            "Submitted velocity command did not apply on the next explicit tick.");
    require(close(entity_by_id(after_tick, scs::domain::EntityId{1}).velocity_km_per_second.y, 0.25),
            "Submitted velocity command y component did not apply on the next explicit tick.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"renderer_shows_map_contacts_selection_and_events", renderer_shows_map_contacts_selection_and_events},
        {"ui_controls_view_pause_time_scale_and_velocity_command", ui_controls_view_pause_time_scale_and_velocity_command},
        {"ui_command_emission_changes_simulation_only_after_submission_and_tick", ui_command_emission_changes_simulation_only_after_submission_and_tick},
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
