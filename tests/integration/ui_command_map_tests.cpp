#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "gameplay/time_scale_policy.h"
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
    snapshot.missile_tracks.push_back(scs::presentation::TacticalMissileTrack{
        scs::domain::MissileId{3},
        scs::domain::EntityId{1},
        scs::domain::ContactId{7},
        scs::domain::Vec2{40.0, 0.0},
        scs::domain::Vec2{25.0, 0.0},
        scs::domain::MissileStatus::InFlight,
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

void renderer_shows_reference_selection_missile_and_time_metrics() {
    const auto snapshot = make_tactical_fixture();
    const auto output = scs::rendering::render_tactical_map(
        snapshot,
        scs::rendering::TacticalMapView{scs::domain::Vec2{0.0, 0.0}, 100.0, 21, 11},
        scs::rendering::TacticalSelection{
            scs::rendering::TacticalSelectionKind::HostileContact,
            scs::domain::EntityId{},
            scs::domain::ContactId{7},
        },
        scs::gameplay::TimeScaleRecommendation{8.0, scs::gameplay::TimeScaleReason::RecentThreat});

    require(output.find("kilometers_per_cell=100.0") != std::string::npos,
            "Renderer did not include kilometers per cell.");
    require(output.find("visible_span=(2100.0 x 1100.0) km") != std::string::npos,
            "Renderer did not include visible span.");
    require(output.find("Orientation: +x east, +y north") != std::string::npos,
            "Renderer did not include map orientation.");
    require(output.find("Time scale=8.0x reason=recent threat event") != std::string::npos,
            "Renderer did not include the time-scale recommendation.");
    require(output.find("Selected contact") != std::string::npos,
            "Renderer did not include selected contact metrics.");
    require(output.find("confidence=0.8 uncertainty=25.0 km age=2 ticks") != std::string::npos,
            "Renderer did not include contact quality metrics.");
    require(output.find("range=223.6 km bearing=63.4 deg") != std::string::npos,
            "Renderer did not include range and bearing.");
    require(output.find("closing_speed=9.8 km/s") != std::string::npos,
            "Renderer did not include closing speed.");
    require(output.find("closest_approach_time=18.2 s") != std::string::npos,
            "Renderer did not include closest approach time.");
    require(output.find("closest_approach_distance=100.0 km") != std::string::npos,
            "Renderer did not include closest approach distance.");
    require(output.find("M3 launcher=F1 target_contact=C7") != std::string::npos,
            "Renderer did not list missile identity and targeting.");
    require(output.find("speed=25.0 km/s status=in_flight") != std::string::npos,
            "Renderer did not include missile speed and status.");
}

void renderer_shows_friendly_selection_metrics() {
    const auto snapshot = make_tactical_fixture();
    const auto output = scs::rendering::render_tactical_map(
        snapshot,
        scs::rendering::TacticalMapView{scs::domain::Vec2{0.0, 0.0}, 100.0, 21, 11},
        scs::rendering::TacticalSelection{
            scs::rendering::TacticalSelectionKind::FriendlyEntity,
            scs::domain::EntityId{1},
            scs::domain::ContactId{},
        });

    require(output.find("Selected friendly") != std::string::npos,
            "Renderer did not include selected friendly metrics.");
    require(output.find("position=(0.0, 0.0) km") != std::string::npos,
            "Renderer did not include friendly position.");
    require(output.find("velocity=(10.0, 0.0) km/s speed=10.0 km/s") != std::string::npos,
            "Renderer did not include friendly velocity and speed.");
    require(output.find("missiles=2 defenses=1") != std::string::npos,
            "Renderer did not include friendly ammunition and defenses.");
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

void ui_emits_engage_contact_for_selected_friendly() {
    const auto snapshot = make_tactical_fixture();
    scs::ui::TacticalUiState state;
    state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::FriendlyEntity,
        scs::domain::EntityId{1},
        scs::domain::ContactId{},
    };

    require(scs::ui::tactical_command_help().find("engage contact <id>") != std::string::npos,
            "Help text did not include engage contact command.");

    const auto result = scs::ui::handle_tactical_input(state, snapshot, "engage contact 7");

    require(result.command.has_value(), "Engage contact command was not emitted.");
    require(result.command->execute_on == snapshot.tick, "Engage contact command used the wrong tick.");
    const auto* payload = std::get_if<scs::domain::EngageContactCommand>(&result.command->payload);
    require(payload != nullptr, "Engage contact emitted the wrong command payload.");
    require(payload->launcher == scs::domain::EntityId{1},
            "Engage contact command used the wrong launcher.");
    require(payload->target == scs::domain::ContactId{7},
            "Engage contact command targeted the wrong contact.");
}

void ui_rejects_engage_contact_without_selected_friendly() {
    const auto snapshot = make_tactical_fixture();
    scs::ui::TacticalUiState state;

    const auto result = scs::ui::handle_tactical_input(state, snapshot, "engage contact 7");

    require(!result.command.has_value(),
            "Engage contact should not emit without a selected friendly entity.");
    require(result.feedback.find("Select a visible friendly entity") != std::string::npos,
            "Engage contact did not explain the missing friendly selection.");
}

void ui_rejects_engage_contact_for_hidden_contact() {
    const auto snapshot = make_tactical_fixture();
    scs::ui::TacticalUiState state;
    state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::FriendlyEntity,
        scs::domain::EntityId{1},
        scs::domain::ContactId{},
    };

    const auto result = scs::ui::handle_tactical_input(state, snapshot, "engage contact 99");

    require(!result.command.has_value(), "Engage contact should not emit for a hidden contact.");
    require(result.feedback.find("Hostile contact is not visible") != std::string::npos,
            "Engage contact did not explain hidden contact rejection.");
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
        {"renderer_shows_reference_selection_missile_and_time_metrics", renderer_shows_reference_selection_missile_and_time_metrics},
        {"renderer_shows_friendly_selection_metrics", renderer_shows_friendly_selection_metrics},
        {"ui_controls_view_pause_time_scale_and_velocity_command", ui_controls_view_pause_time_scale_and_velocity_command},
        {"ui_emits_engage_contact_for_selected_friendly", ui_emits_engage_contact_for_selected_friendly},
        {"ui_rejects_engage_contact_without_selected_friendly", ui_rejects_engage_contact_without_selected_friendly},
        {"ui_rejects_engage_contact_for_hidden_contact", ui_rejects_engage_contact_for_hidden_contact},
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
