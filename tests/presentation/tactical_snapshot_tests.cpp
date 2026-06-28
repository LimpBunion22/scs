#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "presentation/prediction.h"
#include "presentation/tactical_snapshot.h"
#include "simulation/simulation.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool close(double lhs, double rhs) {
    return std::abs(lhs - rhs) < 1e-9;
}

bool same_vec(scs::domain::Vec2 lhs, scs::domain::Vec2 rhs) {
    return close(lhs.x, rhs.x) && close(lhs.y, rhs.y);
}

scs::simulation::Scenario make_presentation_scenario() {
    scs::simulation::Scenario scenario;
    scenario.name = "presentation_test";
    scenario.seed = 0x5c5c2001;
    scenario.fixed_step_seconds = 1.0;

    scenario.entities.push_back(scs::domain::EntityState{
        scs::domain::EntityId{1},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Friendly,
        "Blue Observer",
        scs::domain::Vec2{0.0, 0.0},
        scs::domain::Vec2{2.0, 0.0},
        100.0,
    });

    scenario.entities.push_back(scs::domain::EntityState{
        scs::domain::EntityId{2},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Hostile,
        "Red Target",
        scs::domain::Vec2{50.0, 0.0},
        scs::domain::Vec2{1.0, 0.0},
        0.0,
    });

    return scenario;
}

bool same_contacts(const std::vector<scs::domain::ContactSnapshot>& lhs,
                   const std::vector<scs::domain::ContactSnapshot>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i].id != rhs[i].id ||
            lhs[i].observer != rhs[i].observer ||
            !same_vec(lhs[i].estimated_position_km, rhs[i].estimated_position_km) ||
            !same_vec(lhs[i].estimated_velocity_km_per_second, rhs[i].estimated_velocity_km_per_second) ||
            lhs[i].last_observed_tick != rhs[i].last_observed_tick ||
            !close(lhs[i].confidence, rhs[i].confidence) ||
            lhs[i].classification != rhs[i].classification ||
            !close(lhs[i].uncertainty_radius_km, rhs[i].uncertainty_radius_km)) {
            return false;
        }
    }

    return true;
}

bool same_snapshot(const scs::domain::WorldSnapshot& lhs, const scs::domain::WorldSnapshot& rhs) {
    if (lhs.tick != rhs.tick || !close(lhs.time_seconds, rhs.time_seconds)) {
        return false;
    }

    if (lhs.entities.size() != rhs.entities.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.entities.size(); ++i) {
        if (lhs.entities[i].id != rhs.entities[i].id ||
            lhs.entities[i].kind != rhs.entities[i].kind ||
            lhs.entities[i].allegiance != rhs.entities[i].allegiance ||
            lhs.entities[i].name != rhs.entities[i].name ||
            !same_vec(lhs.entities[i].position_km, rhs.entities[i].position_km) ||
            !same_vec(lhs.entities[i].velocity_km_per_second, rhs.entities[i].velocity_km_per_second)) {
            return false;
        }
    }

    return same_contacts(lhs.contacts, rhs.contacts);
}

bool same_events(const std::vector<scs::domain::Event>& lhs,
                 const std::vector<scs::domain::Event>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i].tick != rhs[i].tick ||
            lhs[i].severity != rhs[i].severity ||
            lhs[i].type != rhs[i].type ||
            lhs[i].subject != rhs[i].subject ||
            lhs[i].message != rhs[i].message) {
            return false;
        }
    }

    return true;
}

const scs::presentation::TacticalTrajectory& trajectory_by_source(
    const scs::presentation::TacticalSnapshot& snapshot,
    scs::presentation::TacticalTrajectorySourceKind source_kind) {
    for (const auto& trajectory : snapshot.predicted_trajectories) {
        if (trajectory.source_kind == source_kind) {
            return trajectory;
        }
    }
    throw std::runtime_error("Missing trajectory for source kind.");
}

void tactical_snapshot_separates_map_data() {
    scs::simulation::Simulation simulation(make_presentation_scenario());

    const auto tactical = scs::presentation::make_tactical_snapshot(
        simulation.snapshot(),
        simulation.events(),
        scs::presentation::TacticalSnapshotOptions{scs::presentation::PredictionConfig{2}});

    require(tactical.tick == 0, "Tactical snapshot tick changed.");
    require(close(tactical.time_seconds, 0.0), "Tactical snapshot time changed.");
    require(tactical.friendly_entities.size() == 1, "Tactical snapshot should expose one friendly entity.");
    require(tactical.friendly_entities.front().id == scs::domain::EntityId{1},
            "Friendly entity id changed.");
    require(tactical.hostile_contacts.size() == 1, "Tactical snapshot should expose one hostile contact.");
    require(tactical.hostile_contacts.front().id == scs::domain::ContactId{1},
            "Hostile contact id changed.");
    require(tactical.events.size() == simulation.events().size(), "Event stream was not copied.");
    require(tactical.predicted_trajectories.size() == 2,
            "Expected trajectories for the friendly entity and hostile contact.");

    const auto& contact_trajectory = trajectory_by_source(
        tactical,
        scs::presentation::TacticalTrajectorySourceKind::HostileContact);
    require(contact_trajectory.contact == scs::domain::ContactId{1},
            "Contact trajectory id changed.");
    require(contact_trajectory.points.size() == 3,
            "Contact prediction did not include the current point plus two future ticks.");
    require(same_vec(contact_trajectory.points.back().position_km, scs::domain::Vec2{52.0, 0.0}),
            "Contact prediction did not use inertial contact velocity.");
}

void prediction_covers_configured_tick_horizon() {
    const auto points = scs::presentation::predict_inertial_trajectory(
        10,
        20.0,
        scs::domain::Vec2{5.0, -2.0},
        scs::domain::Vec2{3.0, 0.5},
        2.0,
        scs::presentation::PredictionConfig{3});

    require(points.size() == 4, "Prediction should include current point plus configured future ticks.");
    require(points.front().tick == 10, "Prediction start tick changed.");
    require(close(points.front().time_seconds, 20.0), "Prediction start time changed.");
    require(same_vec(points.front().position_km, scs::domain::Vec2{5.0, -2.0}),
            "Prediction start position changed.");
    require(points.back().tick == 13, "Prediction horizon tick changed.");
    require(close(points.back().time_seconds, 26.0), "Prediction horizon time changed.");
    require(same_vec(points.back().position_km, scs::domain::Vec2{23.0, 1.0}),
            "Prediction horizon position changed.");
}

void presentation_generation_does_not_mutate_simulation() {
    scs::simulation::Simulation simulation(make_presentation_scenario());
    simulation.advance_one_tick();

    const auto snapshot_before = simulation.snapshot();
    const auto events_before = simulation.events();

    const auto tactical = scs::presentation::make_tactical_snapshot(
        snapshot_before,
        events_before,
        scs::presentation::TacticalSnapshotOptions{scs::presentation::PredictionConfig{4}});

    const auto snapshot_after = simulation.snapshot();
    const auto events_after = simulation.events();

    require(!tactical.predicted_trajectories.empty(),
            "Presentation generation did not produce trajectories.");
    require(same_snapshot(snapshot_before, snapshot_after),
            "Generating presentation data changed simulation snapshot state.");
    require(same_events(events_before, events_after),
            "Generating presentation data changed simulation events.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"tactical_snapshot_separates_map_data", tactical_snapshot_separates_map_data},
        {"prediction_covers_configured_tick_horizon", prediction_covers_configured_tick_horizon},
        {"presentation_generation_does_not_mutate_simulation", presentation_generation_does_not_mutate_simulation},
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
