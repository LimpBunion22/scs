#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "domain/command.h"
#include "gameplay/time_scale_policy.h"
#include "simulation/replay.h"

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

scs::simulation::ReplayInput make_vertical_slice_replay_input() {
    return scs::simulation::ReplayInput{
        scs::simulation::make_playable_engagement_demo_scenario(),
        {
            scs::domain::engage_contact_at(0, scs::domain::EntityId{1}, scs::domain::ContactId{1}),
        },
        4,
    };
}

const scs::domain::EntitySnapshot& entity_by_id(const scs::domain::WorldSnapshot& snapshot,
                                                scs::domain::EntityId id) {
    for (const auto& entity : snapshot.entities) {
        if (entity.id == id) {
            return entity;
        }
    }
    fail("Missing expected entity in final snapshot.");
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

void replay_regression_reproduces_final_snapshot_and_events() {
    const auto input = make_vertical_slice_replay_input();
    require(input.scenario.name == "playable_engagement_demo", "Replay regression scenario changed.");
    require(input.scenario.seed == 0x5c5c0002, "Replay regression seed changed.");
    require(input.commands.size() == 1, "Replay regression command stream changed.");
    require(input.ticks_to_run == 4, "Replay regression tick count changed.");

    const auto first = scs::simulation::run_replay(input);
    const auto second = scs::simulation::run_replay(input);

    require_same_snapshot(first.final_snapshot, second.final_snapshot);
    require_same_events(first.events, second.events);
}

void replay_regression_covers_contact_engagement_time_events_and_final_state() {
    const auto result = scs::simulation::run_replay(make_vertical_slice_replay_input());

    require(result.final_snapshot.tick == 4, "Replay regression final tick changed.");
    require(close(result.final_snapshot.time_seconds, 4.0), "Replay regression final time changed.");
    require(result.final_snapshot.contacts.size() == 1, "Replay regression should retain one contact.");

    const auto& contact = result.final_snapshot.contacts.front();
    require(contact.id == scs::domain::ContactId{1}, "Replay contact id changed.");
    require(contact.observer == scs::domain::EntityId{1}, "Replay contact observer changed.");
    require(contact.last_observed_tick == 4, "Replay contact was not refreshed through the final tick.");
    require(close(contact.confidence, 1.0), "Replay contact confidence changed.");
    require(close(contact.uncertainty_radius_km, 0.0), "Replay contact uncertainty changed.");

    const auto& blue = entity_by_id(result.final_snapshot, scs::domain::EntityId{1});
    const auto& red = entity_by_id(result.final_snapshot, scs::domain::EntityId{2});
    const auto& missile = only_missile(result.final_snapshot);

    require(blue.missile_ammunition == 0, "Replay did not consume Blue missile ammunition.");
    require(red.defensive_response_charges == 0, "Replay did not consume Red defensive response.");
    require(missile.status == scs::domain::MissileStatus::Defeated,
            "Replay missile should be defeated by the defensive response.");
    require(missile.target_entity == scs::domain::EntityId{2}, "Replay missile target entity changed.");
    require(missile.target_contact == scs::domain::ContactId{1}, "Replay missile target contact changed.");

    require(has_event(result.events,
                      0,
                      scs::domain::EventType::ContactDetected,
                      scs::domain::EventSeverity::Advisory,
                      scs::domain::EntityId{1}),
            "Replay did not record contact detection.");
    require(has_event(result.events,
                      0,
                      scs::domain::EventType::MissileLaunched,
                      scs::domain::EventSeverity::Info,
                      scs::domain::EntityId{1}),
            "Replay did not record missile launch.");
    require(has_event(result.events,
                      1,
                      scs::domain::EventType::MissileThreat,
                      scs::domain::EventSeverity::Threat,
                      scs::domain::EntityId{2}),
            "Replay did not record missile threat.");
    require(has_event(result.events,
                      3,
                      scs::domain::EventType::DefensiveResponse,
                      scs::domain::EventSeverity::Critical,
                      scs::domain::EntityId{2}),
            "Replay did not record defensive response.");
    require(has_event(result.events,
                      3,
                      scs::domain::EventType::MissileMissed,
                      scs::domain::EventSeverity::Advisory,
                      scs::domain::EntityId{2}),
            "Replay did not record defended missile miss.");

    const auto recommendation = scs::gameplay::recommend_time_scale(result.final_snapshot.tick, result.events);
    require(recommendation.scale == 1.0, "Critical replay event did not recommend real-time scale.");
    require(recommendation.reason == scs::gameplay::TimeScaleReason::RecentCritical,
            "Critical replay event did not drive the expected time-scale reason.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"replay_regression_reproduces_final_snapshot_and_events",
         replay_regression_reproduces_final_snapshot_and_events},
        {"replay_regression_covers_contact_engagement_time_events_and_final_state",
         replay_regression_covers_contact_engagement_time_events_and_final_state},
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
