#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "domain/command.h"
#include "simulation/replay.h"
#include "simulation/scenario.h"
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

const scs::domain::EntitySnapshot& entity_by_id(const scs::domain::WorldSnapshot& snapshot,
                                                scs::domain::EntityId id) {
    for (const auto& entity : snapshot.entities) {
        if (entity.id == id) {
            return entity;
        }
    }
    throw std::runtime_error("Missing entity in snapshot.");
}

const scs::domain::EntityState& scenario_entity_by_id(const scs::simulation::Scenario& scenario,
                                                      scs::domain::EntityId id) {
    for (const auto& entity : scenario.entities) {
        if (entity.id == id) {
            return entity;
        }
    }
    throw std::runtime_error("Missing entity in scenario.");
}

bool same_snapshot(const scs::domain::WorldSnapshot& lhs, const scs::domain::WorldSnapshot& rhs) {
    if (lhs.tick != rhs.tick || !close(lhs.time_seconds, rhs.time_seconds)) {
        return false;
    }
    if (lhs.entities.size() != rhs.entities.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.entities.size(); ++i) {
        const auto& left = lhs.entities[i];
        const auto& right = rhs.entities[i];
        if (left.id != right.id ||
            left.kind != right.kind ||
            left.allegiance != right.allegiance ||
            left.name != right.name ||
            !same_vec(left.position_km, right.position_km) ||
            !same_vec(left.velocity_km_per_second, right.velocity_km_per_second)) {
            return false;
        }
    }

    return true;
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

void vertical_slice_scenario_initial_state_is_stable() {
    const auto scenario = scs::simulation::make_default_vertical_slice_scenario();

    require(scenario.name == "vertical_slice_core",
            "Default vertical-slice scenario name changed.");
    require(scenario.seed == 0x5c5c0001,
            "Default vertical-slice scenario seed changed.");
    require(close(scenario.fixed_step_seconds, 1.0),
            "Default vertical-slice scenario fixed step changed.");
    require(scenario.entities.size() == 2,
            "Default vertical-slice scenario should contain exactly two combat groups.");

    const auto& blue = scenario_entity_by_id(scenario, scs::domain::EntityId{1});
    require(blue.kind == scs::domain::EntityKind::CombatGroup,
            "Blue entity kind changed.");
    require(blue.allegiance == scs::domain::Allegiance::Friendly,
            "Blue entity allegiance changed.");
    require(blue.name == "Blue Command Group",
            "Blue entity name changed.");
    require(same_vec(blue.position_km, scs::domain::Vec2{-1'000'000.0, 0.0}),
            "Blue initial position changed.");
    require(same_vec(blue.velocity_km_per_second, scs::domain::Vec2{18.0, 0.0}),
            "Blue initial velocity changed.");
    require(close(blue.sensor_range_km, 750'000.0),
            "Blue sensor range changed.");

    const auto& red = scenario_entity_by_id(scenario, scs::domain::EntityId{2});
    require(red.kind == scs::domain::EntityKind::CombatGroup,
            "Red entity kind changed.");
    require(red.allegiance == scs::domain::Allegiance::Hostile,
            "Red entity allegiance changed.");
    require(red.name == "Red Command Group",
            "Red entity name changed.");
    require(same_vec(red.position_km, scs::domain::Vec2{1'000'000.0, 150'000.0}),
            "Red initial position changed.");
    require(same_vec(red.velocity_km_per_second, scs::domain::Vec2{-16.0, -0.25}),
            "Red initial velocity changed.");
    require(close(red.sensor_range_km, 650'000.0),
            "Red sensor range changed.");
}

void vertical_slice_scenario_fixed_tick_positions_are_stable() {
    scs::simulation::Simulation simulation(scs::simulation::make_default_vertical_slice_scenario());

    simulation.advance(120);

    const auto snapshot = simulation.snapshot();
    require(snapshot.tick == 120,
            "Snapshot tick after fixed advance changed.");
    require(close(snapshot.time_seconds, 120.0),
            "Snapshot time after fixed advance changed.");

    const auto& blue = entity_by_id(snapshot, scs::domain::EntityId{1});
    require(same_vec(blue.position_km, scs::domain::Vec2{-997'840.0, 0.0}),
            "Blue fixed-tick position after 120 seconds changed.");

    const auto& red = entity_by_id(snapshot, scs::domain::EntityId{2});
    require(same_vec(red.position_km, scs::domain::Vec2{998'080.0, 149'970.0}),
            "Red fixed-tick position after 120 seconds changed.");
}

void fixed_step_is_independent_of_batching() {
    const auto scenario = scs::simulation::make_default_vertical_slice_scenario();
    scs::simulation::Simulation batched(scenario);
    scs::simulation::Simulation one_by_one(scenario);

    batched.advance(12);
    for (int i = 0; i < 12; ++i) {
        one_by_one.advance_one_tick();
    }

    require(same_snapshot(batched.snapshot(), one_by_one.snapshot()),
            "Fixed-step movement changed when ticks were batched differently.");
}

void replay_reproduces_snapshot_and_events() {
    scs::simulation::ReplayInput input{
        scs::simulation::make_default_vertical_slice_scenario(),
        {
            scs::domain::set_velocity_at(3, scs::domain::EntityId{1}, scs::domain::Vec2{25.0, -0.5}),
            scs::domain::set_velocity_at(7, scs::domain::EntityId{2}, scs::domain::Vec2{-11.0, 0.0}),
        },
        20,
    };

    const auto first = scs::simulation::run_replay(input);
    const auto second = scs::simulation::run_replay(input);

    require(same_snapshot(first.final_snapshot, second.final_snapshot),
            "Replay did not reproduce the same final snapshot.");
    require(same_events(first.events, second.events),
            "Replay did not reproduce the same event stream.");
}

void same_tick_commands_keep_submission_order() {
    const auto scenario = scs::simulation::make_default_vertical_slice_scenario();
    const auto initial = entity_by_id(scs::simulation::Simulation(scenario).snapshot(),
                                      scs::domain::EntityId{1});

    scs::simulation::Simulation simulation(scenario);
    simulation.submit(scs::domain::set_velocity_at(0, scs::domain::EntityId{1}, scs::domain::Vec2{1.0, 0.0}));
    simulation.submit(scs::domain::set_velocity_at(0, scs::domain::EntityId{1}, scs::domain::Vec2{2.0, 0.0}));
    simulation.advance_one_tick();

    const auto snapshot = simulation.snapshot();
    const auto& entity = entity_by_id(snapshot, scs::domain::EntityId{1});

    require(same_vec(entity.velocity_km_per_second, scs::domain::Vec2{2.0, 0.0}),
            "Last same-tick velocity command did not win.");
    require(same_vec(entity.position_km,
                     scs::domain::Vec2{initial.position_km.x + 2.0, initial.position_km.y}),
            "Entity did not move with the final same-tick command velocity.");
}

void invalid_commands_are_rejected() {
    scs::simulation::Simulation simulation(scs::simulation::make_default_vertical_slice_scenario());
    const bool accepted = simulation.submit(
        scs::domain::set_velocity_at(0, scs::domain::EntityId{99}, scs::domain::Vec2{1.0, 1.0}));

    require(!accepted, "Invalid command was accepted.");
    require(!simulation.events().empty(), "Rejected command did not create an event.");
    require(simulation.events().back().type == scs::domain::EventType::CommandRejected,
            "Rejected command event type was not recorded.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"vertical_slice_scenario_initial_state_is_stable", vertical_slice_scenario_initial_state_is_stable},
        {"vertical_slice_scenario_fixed_tick_positions_are_stable", vertical_slice_scenario_fixed_tick_positions_are_stable},
        {"fixed_step_is_independent_of_batching", fixed_step_is_independent_of_batching},
        {"replay_reproduces_snapshot_and_events", replay_reproduces_snapshot_and_events},
        {"same_tick_commands_keep_submission_order", same_tick_commands_keep_submission_order},
        {"invalid_commands_are_rejected", invalid_commands_are_rejected},
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
