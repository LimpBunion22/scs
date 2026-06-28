#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "domain/command.h"
#include "simulation/replay.h"
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

scs::simulation::Scenario make_contact_scenario(double red_position_x,
                                                double red_velocity_x,
                                                double blue_sensor_range = 60.0) {
    scs::simulation::Scenario scenario;
    scenario.name = "contact_test";
    scenario.seed = 0x5c5c1001;
    scenario.fixed_step_seconds = 1.0;

    scenario.entities.push_back(scs::domain::EntityState{
        scs::domain::EntityId{1},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Friendly,
        "Blue Observer",
        scs::domain::Vec2{0.0, 0.0},
        scs::domain::Vec2{0.0, 0.0},
        blue_sensor_range,
    });

    scenario.entities.push_back(scs::domain::EntityState{
        scs::domain::EntityId{2},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Hostile,
        "Red Target",
        scs::domain::Vec2{red_position_x, 0.0},
        scs::domain::Vec2{red_velocity_x, 0.0},
        0.0,
    });

    return scenario;
}

const scs::domain::ContactSnapshot& only_contact(const scs::domain::WorldSnapshot& snapshot) {
    require(snapshot.contacts.size() == 1, "Expected exactly one contact.");
    return snapshot.contacts.front();
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

void friendly_observer_creates_hostile_contact_in_range() {
    scs::simulation::Simulation simulation(make_contact_scenario(50.0, 1.0));

    const auto snapshot = simulation.snapshot();
    const auto& contact = only_contact(snapshot);

    require(contact.id == scs::domain::ContactId{1}, "First contact id changed.");
    require(contact.observer == scs::domain::EntityId{1}, "Contact observer changed.");
    require(same_vec(contact.estimated_position_km, scs::domain::Vec2{50.0, 0.0}),
            "Initial contact estimated position changed.");
    require(same_vec(contact.estimated_velocity_km_per_second, scs::domain::Vec2{1.0, 0.0}),
            "Initial contact estimated velocity changed.");
    require(contact.last_observed_tick == 0, "Initial contact observation tick changed.");
    require(close(contact.confidence, 1.0), "Initial contact confidence changed.");
    require(contact.classification == scs::domain::ContactClassification::HostileCombatGroup,
            "Initial contact classification changed.");
    require(close(contact.uncertainty_radius_km, 0.0), "Initial contact uncertainty changed.");
    require(has_event(simulation.events(),
                      0,
                      scs::domain::EventType::ContactDetected,
                      scs::domain::EventSeverity::Advisory,
                      scs::domain::EntityId{1}),
            "Initial contact detection event was not recorded.");
}

void observed_contact_refreshes_deterministically() {
    scs::simulation::Simulation simulation(make_contact_scenario(50.0, 1.0));

    simulation.advance_one_tick();

    const auto snapshot = simulation.snapshot();
    const auto& contact = only_contact(snapshot);

    require(contact.last_observed_tick == 1, "Observed contact did not refresh on the next tick.");
    require(same_vec(contact.estimated_position_km, scs::domain::Vec2{51.0, 0.0}),
            "Refreshed contact position changed.");
    require(same_vec(contact.estimated_velocity_km_per_second, scs::domain::Vec2{1.0, 0.0}),
            "Refreshed contact velocity changed.");
    require(close(contact.confidence, 1.0), "Refreshed contact confidence changed.");
    require(close(contact.uncertainty_radius_km, 0.0), "Refreshed contact uncertainty did not reset.");
    require(has_event(simulation.events(),
                      1,
                      scs::domain::EventType::ContactUpdated,
                      scs::domain::EventSeverity::Info,
                      scs::domain::EntityId{1}),
            "Contact update event was not recorded.");
}

void stale_contact_uncertainty_grows_each_tick() {
    scs::simulation::Simulation simulation(make_contact_scenario(50.0, 100.0));

    simulation.advance_one_tick();
    const auto first_stale_snapshot = simulation.snapshot();
    const auto& first_stale = only_contact(first_stale_snapshot);

    require(first_stale.last_observed_tick == 0, "Stale contact observation tick changed unexpectedly.");
    require(close(first_stale.uncertainty_radius_km, 100.0), "First stale uncertainty growth changed.");
    require(close(first_stale.confidence, 0.95), "First stale confidence decay changed.");
    require(same_vec(first_stale.estimated_position_km, scs::domain::Vec2{150.0, 0.0}),
            "First stale contact projection changed.");

    simulation.advance_one_tick();
    const auto second_stale_snapshot = simulation.snapshot();
    const auto& second_stale = only_contact(second_stale_snapshot);

    require(second_stale.last_observed_tick == 0, "Second stale observation tick changed unexpectedly.");
    require(second_stale.uncertainty_radius_km > first_stale.uncertainty_radius_km,
            "Stale contact uncertainty did not grow monotonically.");
    require(close(second_stale.uncertainty_radius_km, 200.0), "Second stale uncertainty growth changed.");
    require(second_stale.confidence < first_stale.confidence,
            "Stale contact confidence did not decrease.");
    require(same_vec(second_stale.estimated_position_km, scs::domain::Vec2{250.0, 0.0}),
            "Second stale contact projection changed.");
}

void fresh_observation_resets_stale_uncertainty() {
    scs::simulation::Simulation simulation(make_contact_scenario(50.0, 100.0));

    simulation.advance_one_tick();
    const auto stale_snapshot = simulation.snapshot();
    const auto stale_uncertainty = only_contact(stale_snapshot).uncertainty_radius_km;

    simulation.submit(scs::domain::set_velocity_at(simulation.current_tick(),
                                                   scs::domain::EntityId{2},
                                                   scs::domain::Vec2{-100.0, 0.0}));
    simulation.advance_one_tick();

    const auto refreshed_snapshot = simulation.snapshot();
    const auto& refreshed = only_contact(refreshed_snapshot);

    require(refreshed.last_observed_tick == 2, "Reobserved contact tick changed.");
    require(refreshed.uncertainty_radius_km < stale_uncertainty,
            "Reobserved contact uncertainty did not reduce.");
    require(close(refreshed.uncertainty_radius_km, 0.0), "Reobserved contact uncertainty did not reset.");
    require(close(refreshed.confidence, 1.0), "Reobserved contact confidence did not reset.");
    require(same_vec(refreshed.estimated_position_km, scs::domain::Vec2{50.0, 0.0}),
            "Reobserved contact position changed.");
    require(has_event(simulation.events(),
                      2,
                      scs::domain::EventType::ContactUpdated,
                      scs::domain::EventSeverity::Info,
                      scs::domain::EntityId{1}),
            "Reobserved contact update event was not recorded.");
}

void replay_reproduces_contact_snapshots_and_events() {
    scs::simulation::ReplayInput input{
        make_contact_scenario(50.0, 100.0),
        {
            scs::domain::set_velocity_at(1, scs::domain::EntityId{2}, scs::domain::Vec2{-100.0, 0.0}),
        },
        4,
    };

    const auto first = scs::simulation::run_replay(input);
    const auto second = scs::simulation::run_replay(input);

    require(same_snapshot(first.final_snapshot, second.final_snapshot),
            "Replay did not reproduce the same contact snapshot.");
    require(same_events(first.events, second.events),
            "Replay did not reproduce the same contact event stream.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"friendly_observer_creates_hostile_contact_in_range", friendly_observer_creates_hostile_contact_in_range},
        {"observed_contact_refreshes_deterministically", observed_contact_refreshes_deterministically},
        {"stale_contact_uncertainty_grows_each_tick", stale_contact_uncertainty_grows_each_tick},
        {"fresh_observation_resets_stale_uncertainty", fresh_observation_resets_stale_uncertainty},
        {"replay_reproduces_contact_snapshots_and_events", replay_reproduces_contact_snapshots_and_events},
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
