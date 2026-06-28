#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "gameplay/time_scale_policy.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

scs::domain::Event make_event(scs::domain::Tick tick,
                              scs::domain::EventSeverity severity,
                              scs::domain::EventType type = scs::domain::EventType::ScenarioLoaded) {
    return scs::domain::Event{tick, severity, type, scs::domain::EntityId{1}, "test event"};
}

void quiet_time_uses_high_acceleration() {
    const auto recommendation = scs::gameplay::recommend_time_scale(100, {});

    require(recommendation.scale == 1024.0,
            "Quiet time scale changed.");
    require(recommendation.reason == scs::gameplay::TimeScaleReason::Quiet,
            "Quiet time reason changed.");
    require(std::string(scs::gameplay::time_scale_reason_label(recommendation.reason)) == "quiet",
            "Quiet time reason label changed.");
}

void old_events_are_ignored() {
    const std::vector<scs::domain::Event> events{
        make_event(69, scs::domain::EventSeverity::Critical),
        make_event(70, scs::domain::EventSeverity::Info),
    };

    const auto recommendation = scs::gameplay::recommend_time_scale(100, events);

    require(recommendation.scale == 256.0,
            "Event at the inclusive recent-window boundary was not considered.");
    require(recommendation.reason == scs::gameplay::TimeScaleReason::RecentInfo,
            "Recent boundary event reason changed.");
}

void severity_mapping_is_stable() {
    struct Case {
        scs::domain::EventSeverity severity;
        double expected_scale;
        scs::gameplay::TimeScaleReason expected_reason;
    };

    const std::vector<Case> cases{
        {scs::domain::EventSeverity::Info, 256.0, scs::gameplay::TimeScaleReason::RecentInfo},
        {scs::domain::EventSeverity::Advisory, 64.0, scs::gameplay::TimeScaleReason::RecentAdvisory},
        {scs::domain::EventSeverity::Threat, 8.0, scs::gameplay::TimeScaleReason::RecentThreat},
        {scs::domain::EventSeverity::Critical, 1.0, scs::gameplay::TimeScaleReason::RecentCritical},
    };

    for (const auto& test_case : cases) {
        const auto recommendation =
            scs::gameplay::recommend_time_scale(12, {make_event(12, test_case.severity)});

        require(recommendation.scale == test_case.expected_scale,
                "Severity-to-scale mapping changed.");
        require(recommendation.reason == test_case.expected_reason,
                "Severity-to-reason mapping changed.");
    }
}

void contact_activity_uses_advisory_scale() {
    const std::vector<scs::domain::Event> events{
        make_event(10, scs::domain::EventSeverity::Info, scs::domain::EventType::ContactUpdated),
        make_event(10, scs::domain::EventSeverity::Advisory, scs::domain::EventType::ContactDetected),
    };

    const auto recommendation = scs::gameplay::recommend_time_scale(10, events);

    require(recommendation.scale == 64.0,
            "Contact advisory time scale changed.");
    require(recommendation.reason == scs::gameplay::TimeScaleReason::RecentAdvisory,
            "Contact advisory reason changed.");
}

void most_severe_recent_event_wins() {
    const std::vector<scs::domain::Event> events{
        make_event(15, scs::domain::EventSeverity::Info),
        make_event(16, scs::domain::EventSeverity::Critical),
        make_event(17, scs::domain::EventSeverity::Threat),
    };

    const auto recommendation = scs::gameplay::recommend_time_scale(17, events);

    require(recommendation.scale == 1.0,
            "Critical event did not force real-time scale.");
    require(recommendation.reason == scs::gameplay::TimeScaleReason::RecentCritical,
            "Critical event reason changed.");
}

void player_override_takes_precedence() {
    const scs::gameplay::PlayerTimeScaleInput player_input{false, 12.5};
    const std::vector<scs::domain::Event> events{
        make_event(1, scs::domain::EventSeverity::Critical),
    };

    const auto recommendation = scs::gameplay::recommend_time_scale(1, events, player_input);

    require(recommendation.scale == 12.5,
            "Player override scale was not used.");
    require(recommendation.reason == scs::gameplay::TimeScaleReason::PlayerOverride,
            "Player override reason changed.");
}

void tactical_pause_takes_precedence() {
    const scs::gameplay::PlayerTimeScaleInput player_input{true, 12.5};
    const std::vector<scs::domain::Event> events{
        make_event(1, scs::domain::EventSeverity::Critical),
    };

    const auto recommendation = scs::gameplay::recommend_time_scale(1, events, player_input);

    require(recommendation.scale == 0.0,
            "Tactical pause did not recommend zero scale.");
    require(recommendation.reason == scs::gameplay::TimeScaleReason::TacticalPause,
            "Tactical pause reason changed.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"quiet_time_uses_high_acceleration", quiet_time_uses_high_acceleration},
        {"old_events_are_ignored", old_events_are_ignored},
        {"severity_mapping_is_stable", severity_mapping_is_stable},
        {"contact_activity_uses_advisory_scale", contact_activity_uses_advisory_scale},
        {"most_severe_recent_event_wins", most_severe_recent_event_wins},
        {"player_override_takes_precedence", player_override_takes_precedence},
        {"tactical_pause_takes_precedence", tactical_pause_takes_precedence},
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
