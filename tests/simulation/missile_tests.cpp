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

scs::simulation::Scenario make_missile_scenario(int blue_ammunition,
                                                int red_defenses,
                                                double red_position_x = 350.0,
                                                double blue_sensor_range = 0.0) {
    scs::simulation::Scenario scenario;
    scenario.name = "missile_test";
    scenario.seed = 0x5c5c2001;
    scenario.fixed_step_seconds = 1.0;

    scenario.entities.push_back(scs::domain::EntityState{
        scs::domain::EntityId{1},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Friendly,
        "Blue Shooter",
        scs::domain::Vec2{0.0, 0.0},
        scs::domain::Vec2{0.0, 0.0},
        blue_sensor_range,
        blue_ammunition,
        0,
    });

    scenario.entities.push_back(scs::domain::EntityState{
        scs::domain::EntityId{2},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Hostile,
        "Red Target",
        scs::domain::Vec2{red_position_x, 0.0},
        scs::domain::Vec2{0.0, 0.0},
        0.0,
        0,
        red_defenses,
    });

    return scenario;
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

const scs::domain::MissileSnapshot& only_missile(const scs::domain::WorldSnapshot& snapshot) {
    require(snapshot.missiles.size() == 1, "Expected exactly one missile track.");
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

bool same_missiles(const std::vector<scs::domain::MissileSnapshot>& lhs,
                   const std::vector<scs::domain::MissileSnapshot>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i].id != rhs[i].id ||
            lhs[i].launcher != rhs[i].launcher ||
            lhs[i].target_entity != rhs[i].target_entity ||
            lhs[i].target_contact != rhs[i].target_contact ||
            !same_vec(lhs[i].position_km, rhs[i].position_km) ||
            !same_vec(lhs[i].velocity_km_per_second, rhs[i].velocity_km_per_second) ||
            lhs[i].status != rhs[i].status) {
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
        const auto& left = lhs.entities[i];
        const auto& right = rhs.entities[i];
        if (left.id != right.id ||
            left.kind != right.kind ||
            left.allegiance != right.allegiance ||
            left.name != right.name ||
            !same_vec(left.position_km, right.position_km) ||
            !same_vec(left.velocity_km_per_second, right.velocity_km_per_second) ||
            left.missile_ammunition != right.missile_ammunition ||
            left.defensive_response_charges != right.defensive_response_charges) {
            return false;
        }
    }

    return same_contacts(lhs.contacts, rhs.contacts) && same_missiles(lhs.missiles, rhs.missiles);
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

void valid_entity_engagement_launches_one_missile() {
    scs::simulation::Simulation simulation(make_missile_scenario(1, 0));

    require(simulation.submit(
                scs::domain::engage_entity_at(0, scs::domain::EntityId{1}, scs::domain::EntityId{2})),
            "Valid engagement command was not accepted.");
    simulation.advance_one_tick();

    const auto snapshot = simulation.snapshot();
    const auto& blue = entity_by_id(snapshot, scs::domain::EntityId{1});
    const auto& missile = only_missile(snapshot);

    require(blue.missile_ammunition == 0, "Missile ammunition was not consumed on launch.");
    require(missile.id == scs::domain::MissileId{1}, "First missile id changed.");
    require(missile.launcher == scs::domain::EntityId{1}, "Missile launcher changed.");
    require(missile.target_entity == scs::domain::EntityId{2}, "Missile target entity changed.");
    require(missile.target_contact == scs::domain::ContactId{}, "Entity-targeted missile should not record a contact.");
    require(missile.status == scs::domain::MissileStatus::InFlight, "Missile resolved too early.");
    require(same_vec(missile.position_km, scs::domain::Vec2{100.0, 0.0}), "First missile movement changed.");
    require(same_vec(missile.velocity_km_per_second, scs::domain::Vec2{100.0, 0.0}),
            "Missile velocity changed.");
    require(has_event(simulation.events(),
                      0,
                      scs::domain::EventType::MissileLaunched,
                      scs::domain::EventSeverity::Info,
                      scs::domain::EntityId{1}),
            "Missile launch event was not recorded.");
    require(has_event(simulation.events(),
                      1,
                      scs::domain::EventType::MissileThreat,
                      scs::domain::EventSeverity::Threat,
                      scs::domain::EntityId{2}),
            "Missile threat event was not recorded.");
}

void engagement_without_ammunition_is_rejected() {
    scs::simulation::Simulation simulation(make_missile_scenario(0, 0));

    require(simulation.submit(
                scs::domain::engage_entity_at(0, scs::domain::EntityId{1}, scs::domain::EntityId{2})),
            "Engagement command with known launcher should be accepted for execution.");
    simulation.advance_one_tick();

    const auto snapshot = simulation.snapshot();
    require(snapshot.missiles.empty(), "Missile launched without ammunition.");
    require(has_event(simulation.events(),
                      0,
                      scs::domain::EventType::CommandRejected,
                      scs::domain::EventSeverity::Advisory,
                      scs::domain::EntityId{1}),
            "No-ammunition engagement rejection was not recorded.");
}

void contact_engagement_records_contact_and_target_entity() {
    scs::simulation::Simulation simulation(make_missile_scenario(1, 0, 350.0, 500.0));
    const auto contact = simulation.snapshot().contacts.front();

    require(simulation.submit(
                scs::domain::engage_contact_at(0, scs::domain::EntityId{1}, contact.id)),
            "Contact engagement command was not accepted.");
    simulation.advance_one_tick();

    const auto snapshot = simulation.snapshot();
    const auto& missile = only_missile(snapshot);
    require(missile.target_contact == contact.id, "Contact-targeted missile did not record the contact id.");
    require(missile.target_entity == scs::domain::EntityId{2}, "Contact-targeted missile resolved the wrong entity.");
}

void defensive_response_changes_the_deterministic_outcome() {
    scs::simulation::Simulation undefended(make_missile_scenario(1, 0));
    undefended.submit(scs::domain::engage_entity_at(0, scs::domain::EntityId{1}, scs::domain::EntityId{2}));
    undefended.advance(4);

    const auto undefended_snapshot = undefended.snapshot();
    require(only_missile(undefended_snapshot).status == scs::domain::MissileStatus::Hit,
            "Undefended target should be hit.");
    require(has_event(undefended.events(),
                      4,
                      scs::domain::EventType::MissileHit,
                      scs::domain::EventSeverity::Critical,
                      scs::domain::EntityId{2}),
            "Missile hit event was not recorded.");

    scs::simulation::Simulation defended(make_missile_scenario(1, 1));
    defended.submit(scs::domain::engage_entity_at(0, scs::domain::EntityId{1}, scs::domain::EntityId{2}));
    defended.advance(4);

    const auto defended_snapshot = defended.snapshot();
    const auto& red = entity_by_id(defended_snapshot, scs::domain::EntityId{2});
    require(only_missile(defended_snapshot).status == scs::domain::MissileStatus::Defeated,
            "Defended target should defeat the missile.");
    require(red.defensive_response_charges == 0, "Defensive response charge was not consumed.");
    require(has_event(defended.events(),
                      3,
                      scs::domain::EventType::DefensiveResponse,
                      scs::domain::EventSeverity::Critical,
                      scs::domain::EntityId{2}),
            "Defensive response event was not recorded.");
    require(has_event(defended.events(),
                      3,
                      scs::domain::EventType::MissileMissed,
                      scs::domain::EventSeverity::Advisory,
                      scs::domain::EntityId{2}),
            "Defended missile miss event was not recorded.");
}

void replay_reproduces_missile_snapshot_and_events() {
    scs::simulation::ReplayInput input{
        make_missile_scenario(1, 1, 350.0, 500.0),
        {
            scs::domain::engage_contact_at(0, scs::domain::EntityId{1}, scs::domain::ContactId{1}),
        },
        4,
    };

    const auto first = scs::simulation::run_replay(input);
    const auto second = scs::simulation::run_replay(input);

    require(same_snapshot(first.final_snapshot, second.final_snapshot),
            "Replay did not reproduce the same missile snapshot.");
    require(same_events(first.events, second.events),
            "Replay did not reproduce the same missile event stream.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"valid_entity_engagement_launches_one_missile", valid_entity_engagement_launches_one_missile},
        {"engagement_without_ammunition_is_rejected", engagement_without_ammunition_is_rejected},
        {"contact_engagement_records_contact_and_target_entity", contact_engagement_records_contact_and_target_entity},
        {"defensive_response_changes_the_deterministic_outcome", defensive_response_changes_the_deterministic_outcome},
        {"replay_reproduces_missile_snapshot_and_events", replay_reproduces_missile_snapshot_and_events},
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
