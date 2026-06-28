#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "gameplay/time_scale_policy.h"
#include "presentation/tactical_snapshot.h"
#include "ui/desktop_interaction.h"
#include "ui/desktop_panel_model.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

const std::string& metric_value(const std::vector<scs::ui::DesktopPanelMetric>& metrics,
                                const std::string& label) {
    for (const auto& metric : metrics) {
        if (metric.label == label) {
            return metric.value;
        }
    }

    throw std::runtime_error("Missing metric: " + label);
}

const scs::ui::DesktopPanelStagedStatus& staged_status(
    const std::vector<scs::ui::DesktopPanelStagedStatus>& statuses,
    const std::string& label) {
    for (const auto& status : statuses) {
        if (status.label == label) {
            return status;
        }
    }

    throw std::runtime_error("Missing staged status: " + label);
}

scs::presentation::TacticalSnapshot make_panel_fixture() {
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
    snapshot.missile_tracks.push_back(scs::presentation::TacticalMissileTrack{
        scs::domain::MissileId{3},
        scs::domain::EntityId{1},
        scs::domain::ContactId{7},
        scs::domain::Vec2{40.0, 0.0},
        scs::domain::Vec2{25.0, 0.0},
        scs::domain::MissileStatus::InFlight,
    });
    snapshot.events.push_back(scs::domain::Event{
        42,
        scs::domain::EventSeverity::Advisory,
        scs::domain::EventType::ContactDetected,
        scs::domain::EntityId{1},
        "Contact detected for test.",
    });
    return snapshot;
}

void friendly_selection_exposes_speed_ammunition_and_ready_staging() {
    const auto snapshot = make_panel_fixture();
    scs::ui::TacticalUiState ui_state;
    ui_state.command_log.push_back("Staged engagement ready.");

    scs::ui::DesktopInteractionState desktop_state;
    desktop_state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::FriendlyEntity,
        scs::domain::EntityId{1},
        scs::domain::ContactId{},
    };
    desktop_state.staged_launcher = scs::domain::EntityId{1};
    desktop_state.staged_target = scs::domain::ContactId{7};
    desktop_state.hover = scs::ui::DesktopMapObject{
        scs::ui::DesktopMapObjectKind::FriendlyEntity,
        scs::domain::EntityId{1},
        scs::domain::ContactId{},
        scs::domain::MissileId{},
    };

    const auto model = scs::ui::make_desktop_panel_model(
        ui_state,
        desktop_state,
        snapshot,
        scs::gameplay::TimeScaleRecommendation{
            8.0,
            scs::gameplay::TimeScaleReason::RecentThreat,
        });

    require(model.selection.heading == "F1 Blue Fixture",
            "Friendly selection heading changed.");
    require(metric_value(model.selection.metrics, "Speed") == "10.0 km/s",
            "Friendly speed metric changed.");
    require(metric_value(model.selection.metrics, "Missiles") == "2",
            "Friendly missile count changed.");
    require(metric_value(model.selection.metrics, "Defenses") == "1",
            "Friendly defense count changed.");
    require(model.hover_summary == "F1 Blue Fixture",
            "Hover summary changed.");
    require(model.time_scale_label == "8.0x",
            "Time-scale label changed.");
    require(model.time_scale_reason == "recent threat event",
            "Time-scale reason changed.");

    const auto& launcher = staged_status(model.staged_statuses, "Launcher");
    require(launcher.value == "F1 Blue Fixture", "Staged launcher value changed.");
    require(launcher.status == "ready", "Visible staged launcher should be ready.");
    const auto& target = staged_status(model.staged_statuses, "Target");
    require(target.value == "C7", "Staged target value changed.");
    require(target.status == "ready", "Visible staged target should be ready.");

    require(model.command_log_lines.size() == 1 &&
                model.command_log_lines.front() == "Staged engagement ready.",
            "Command log lines were not copied.");
    require(model.event_lines.size() == 1 &&
                model.event_lines.front().find("contact_detected") != std::string::npos,
            "Event lines were not formatted.");

    require(model.missiles.size() == 1, "Expected one missile model.");
    require(model.missiles.front().heading == "M3", "Missile heading changed.");
    require(metric_value(model.missiles.front().metrics, "Launcher") == "F1",
            "Missile launcher metric changed.");
    require(metric_value(model.missiles.front().metrics, "Target") == "C7",
            "Missile target metric changed.");
    require(metric_value(model.missiles.front().metrics, "Speed") == "25.0 km/s",
            "Missile speed metric changed.");
    require(metric_value(model.missiles.front().metrics, "Status") == "in_flight",
            "Missile status metric changed.");
}

void contact_selection_exposes_range_bearing_and_closest_approach() {
    const auto snapshot = make_panel_fixture();
    scs::ui::TacticalUiState ui_state;
    scs::ui::DesktopInteractionState desktop_state;
    desktop_state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::HostileContact,
        scs::domain::EntityId{},
        scs::domain::ContactId{7},
    };

    const auto model = scs::ui::make_desktop_panel_model(
        ui_state,
        desktop_state,
        snapshot,
        scs::gameplay::TimeScaleRecommendation{1.0, scs::gameplay::TimeScaleReason::Quiet});

    require(model.selection.heading == "C7", "Contact selection heading changed.");
    require(metric_value(model.selection.metrics, "Confidence") == "0.8",
            "Contact confidence metric changed.");
    require(metric_value(model.selection.metrics, "Uncertainty") == "25.0 km",
            "Contact uncertainty metric changed.");
    require(metric_value(model.selection.metrics, "Age") == "2 ticks",
            "Contact age metric changed.");
    require(metric_value(model.selection.metrics, "Range") == "223.6 km",
            "Contact range metric changed.");
    require(metric_value(model.selection.metrics, "Bearing") == "63.4 deg",
            "Contact bearing metric changed.");
    require(metric_value(model.selection.metrics, "Closing Speed") == "9.8 km/s",
            "Contact closing speed metric changed.");
    require(metric_value(model.selection.metrics, "Closest Approach Time") == "18.2 s",
            "Closest approach time changed.");
    require(metric_value(model.selection.metrics, "Closest Approach Distance") == "100.0 km",
            "Closest approach distance changed.");
}

void unknown_metrics_are_used_when_observer_or_target_are_unavailable() {
    auto snapshot = make_panel_fixture();
    snapshot.hostile_contacts.front().observer = scs::domain::EntityId{99};
    snapshot.missile_tracks.front().target_contact = scs::domain::ContactId{};

    scs::ui::TacticalUiState ui_state;
    scs::ui::DesktopInteractionState desktop_state;
    desktop_state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::HostileContact,
        scs::domain::EntityId{},
        scs::domain::ContactId{7},
    };
    desktop_state.staged_launcher = scs::domain::EntityId{99};
    desktop_state.staged_target = scs::domain::ContactId{9};
    desktop_state.hover = scs::ui::DesktopMapObject{
        scs::ui::DesktopMapObjectKind::MissileTrack,
        scs::domain::EntityId{},
        scs::domain::ContactId{},
        scs::domain::MissileId{99},
    };

    const auto model = scs::ui::make_desktop_panel_model(
        ui_state,
        desktop_state,
        snapshot,
        scs::gameplay::TimeScaleRecommendation{1.0, scs::gameplay::TimeScaleReason::Quiet});

    require(metric_value(model.selection.metrics, "Range") == "unknown",
            "Contact range should be unknown without a visible observer.");
    require(metric_value(model.selection.metrics, "Bearing") == "unknown",
            "Contact bearing should be unknown without a visible observer.");
    require(metric_value(model.selection.metrics, "Closing Speed") == "unknown",
            "Contact closing speed should be unknown without a visible observer.");
    require(metric_value(model.selection.metrics, "Closest Approach Time") == "unknown",
            "Closest approach time should be unknown without a visible observer.");
    require(metric_value(model.selection.metrics, "Closest Approach Distance") == "unknown",
            "Closest approach distance should be unknown without a visible observer.");
    require(metric_value(model.missiles.front().metrics, "Target") == "unknown",
            "Missile target should be unknown when no target contact is visible.");

    const auto& launcher = staged_status(model.staged_statuses, "Launcher");
    require(launcher.status == "not visible",
            "Missing staged launcher should be marked not visible.");
    const auto& target = staged_status(model.staged_statuses, "Target");
    require(target.status == "not visible",
            "Missing staged target should be marked not visible.");
    require(model.hover_summary == "M99 unavailable",
            "Missing hovered missile should be marked unavailable.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"friendly_selection_exposes_speed_ammunition_and_ready_staging",
         friendly_selection_exposes_speed_ammunition_and_ready_staging},
        {"contact_selection_exposes_range_bearing_and_closest_approach",
         contact_selection_exposes_range_bearing_and_closest_approach},
        {"unknown_metrics_are_used_when_observer_or_target_are_unavailable",
         unknown_metrics_are_used_when_observer_or_target_are_unavailable},
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
