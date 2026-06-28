#include "ui/desktop_panel_model.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace scs::ui {
namespace {

constexpr std::size_t kEventLogLimit = 10;
constexpr double kPi = 3.14159265358979323846;

std::string number(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    return stream.str();
}

double magnitude(domain::Vec2 value) {
    return std::sqrt(domain::magnitude_squared(value));
}

double dot(domain::Vec2 lhs, domain::Vec2 rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

double bearing_degrees(domain::Vec2 offset) {
    double bearing = std::atan2(offset.x, offset.y) * 180.0 / kPi;
    if (bearing < 0.0) {
        bearing += 360.0;
    }
    return bearing;
}

const char* severity_label(domain::EventSeverity severity) {
    switch (severity) {
    case domain::EventSeverity::Info:
        return "info";
    case domain::EventSeverity::Advisory:
        return "advisory";
    case domain::EventSeverity::Threat:
        return "threat";
    case domain::EventSeverity::Critical:
        return "critical";
    }
    return "unknown";
}

const char* event_type_label(domain::EventType type) {
    switch (type) {
    case domain::EventType::ScenarioLoaded:
        return "scenario_loaded";
    case domain::EventType::CommandAccepted:
        return "command_accepted";
    case domain::EventType::CommandRejected:
        return "command_rejected";
    case domain::EventType::VelocityChanged:
        return "velocity_changed";
    case domain::EventType::ContactDetected:
        return "contact_detected";
    case domain::EventType::ContactUpdated:
        return "contact_updated";
    case domain::EventType::MissileLaunched:
        return "missile_launched";
    case domain::EventType::MissileThreat:
        return "missile_threat";
    case domain::EventType::DefensiveResponse:
        return "defensive_response";
    case domain::EventType::MissileHit:
        return "missile_hit";
    case domain::EventType::MissileMissed:
        return "missile_missed";
    }
    return "unknown";
}

const char* missile_status_label(domain::MissileStatus status) {
    switch (status) {
    case domain::MissileStatus::InFlight:
        return "in_flight";
    case domain::MissileStatus::Hit:
        return "hit";
    case domain::MissileStatus::Miss:
        return "miss";
    case domain::MissileStatus::Defeated:
        return "defeated";
    }
    return "unknown";
}

const domain::EntitySnapshot* find_friendly(const presentation::TacticalSnapshot& snapshot,
                                            domain::EntityId id) {
    for (const auto& entity : snapshot.friendly_entities) {
        if (entity.id == id) {
            return &entity;
        }
    }
    return nullptr;
}

const domain::ContactSnapshot* find_contact(const presentation::TacticalSnapshot& snapshot,
                                            domain::ContactId id) {
    for (const auto& contact : snapshot.hostile_contacts) {
        if (contact.id == id) {
            return &contact;
        }
    }
    return nullptr;
}

std::string entity_id_label(domain::EntityId id) {
    return "F" + std::to_string(id.value);
}

std::string entity_label(const domain::EntitySnapshot& entity) {
    return entity_id_label(entity.id) + " " + entity.name;
}

std::string contact_label(domain::ContactId id) {
    return "C" + std::to_string(id.value);
}

std::string missile_label(domain::MissileId id) {
    return "M" + std::to_string(id.value);
}

std::string format_position(domain::Vec2 position_km) {
    return number(position_km.x) + ", " + number(position_km.y) + " km";
}

std::string format_velocity(domain::Vec2 velocity_km_per_second) {
    return number(velocity_km_per_second.x) + ", " +
           number(velocity_km_per_second.y) + " km/s";
}

DesktopPanelSelectionModel make_empty_selection() {
    DesktopPanelSelectionModel model;
    model.heading = "None";
    return model;
}

DesktopPanelSelectionModel make_friendly_selection(
    const presentation::TacticalSnapshot& snapshot,
    domain::EntityId entity_id) {
    DesktopPanelSelectionModel model;
    model.kind = rendering::TacticalSelectionKind::FriendlyEntity;

    const auto* entity = find_friendly(snapshot, entity_id);
    if (entity == nullptr) {
        model.heading = "Friendly unavailable";
        model.metrics.push_back({"Availability", entity_id_label(entity_id) + " not visible"});
        return model;
    }

    model.heading = entity_label(*entity);
    model.metrics.push_back({"Position", format_position(entity->position_km)});
    model.metrics.push_back({"Velocity", format_velocity(entity->velocity_km_per_second)});
    model.metrics.push_back({"Speed", number(magnitude(entity->velocity_km_per_second)) + " km/s"});
    model.metrics.push_back({"Missiles", std::to_string(entity->missile_ammunition)});
    model.metrics.push_back({"Defenses", std::to_string(entity->defensive_response_charges)});
    return model;
}

void append_unknown_contact_observer_metrics(DesktopPanelSelectionModel& model) {
    model.metrics.push_back({"Range", "unknown"});
    model.metrics.push_back({"Bearing", "unknown"});
    model.metrics.push_back({"Closing Speed", "unknown"});
    model.metrics.push_back({"Closest Approach Time", "unknown"});
    model.metrics.push_back({"Closest Approach Distance", "unknown"});
}

DesktopPanelSelectionModel make_contact_selection(
    const presentation::TacticalSnapshot& snapshot,
    domain::ContactId contact_id) {
    DesktopPanelSelectionModel model;
    model.kind = rendering::TacticalSelectionKind::HostileContact;

    const auto* contact = find_contact(snapshot, contact_id);
    if (contact == nullptr) {
        model.heading = "Contact unavailable";
        model.metrics.push_back({"Availability", contact_label(contact_id) + " not visible"});
        return model;
    }

    model.heading = contact_label(contact->id);
    model.metrics.push_back({"Observer", entity_id_label(contact->observer)});
    model.metrics.push_back({"Estimate", format_position(contact->estimated_position_km)});
    model.metrics.push_back({"Velocity", format_velocity(contact->estimated_velocity_km_per_second)});
    model.metrics.push_back({"Confidence", number(contact->confidence)});
    model.metrics.push_back({"Uncertainty", number(contact->uncertainty_radius_km) + " km"});
    if (snapshot.tick >= contact->last_observed_tick) {
        model.metrics.push_back(
            {"Age", std::to_string(snapshot.tick - contact->last_observed_tick) + " ticks"});
    } else {
        model.metrics.push_back({"Age", "unknown"});
    }

    const auto* observer = find_friendly(snapshot, contact->observer);
    if (observer == nullptr) {
        append_unknown_contact_observer_metrics(model);
        return model;
    }

    const domain::Vec2 relative_position = contact->estimated_position_km - observer->position_km;
    const domain::Vec2 relative_velocity =
        contact->estimated_velocity_km_per_second - observer->velocity_km_per_second;
    const double range_km = magnitude(relative_position);
    const double relative_speed_squared = domain::magnitude_squared(relative_velocity);
    const double closest_time_seconds =
        relative_speed_squared == 0.0
            ? 0.0
            : std::max(0.0, -dot(relative_position, relative_velocity) / relative_speed_squared);
    const domain::Vec2 closest_offset =
        relative_position + relative_velocity * closest_time_seconds;

    model.metrics.push_back({"Range", number(range_km) + " km"});
    model.metrics.push_back(
        {"Bearing",
         range_km == 0.0 ? std::string("unknown")
                         : number(bearing_degrees(relative_position)) + " deg"});
    model.metrics.push_back(
        {"Closing Speed",
         range_km == 0.0 ? std::string("unknown")
                         : number(-dot(relative_position, relative_velocity) / range_km) + " km/s"});
    model.metrics.push_back(
        {"Closest Approach Time", number(closest_time_seconds) + " s"});
    model.metrics.push_back(
        {"Closest Approach Distance", number(magnitude(closest_offset)) + " km"});
    return model;
}

DesktopPanelSelectionModel make_selection_model(
    const presentation::TacticalSnapshot& snapshot,
    const DesktopInteractionState& desktop_state) {
    switch (desktop_state.selection.kind) {
    case rendering::TacticalSelectionKind::FriendlyEntity:
        return make_friendly_selection(snapshot, desktop_state.selection.entity);
    case rendering::TacticalSelectionKind::HostileContact:
        return make_contact_selection(snapshot, desktop_state.selection.contact);
    case rendering::TacticalSelectionKind::None:
        return make_empty_selection();
    }

    return make_empty_selection();
}

DesktopPanelStagedStatus make_staged_launcher_status(
    const presentation::TacticalSnapshot& snapshot,
    domain::EntityId launcher_id) {
    DesktopPanelStagedStatus status;
    status.label = "Launcher";

    if (!domain::is_valid(launcher_id)) {
        status.value = "none";
        status.status = "not staged";
        return status;
    }

    const auto* entity = find_friendly(snapshot, launcher_id);
    if (entity == nullptr) {
        status.value = entity_id_label(launcher_id);
        status.status = "not visible";
        return status;
    }

    status.value = entity_label(*entity);
    status.status = "ready";
    return status;
}

DesktopPanelStagedStatus make_staged_target_status(
    const presentation::TacticalSnapshot& snapshot,
    domain::ContactId target_id) {
    DesktopPanelStagedStatus status;
    status.label = "Target";

    if (!domain::is_valid(target_id)) {
        status.value = "none";
        status.status = "not staged";
        return status;
    }

    const auto* contact = find_contact(snapshot, target_id);
    if (contact == nullptr) {
        status.value = contact_label(target_id);
        status.status = "not visible";
        return status;
    }

    status.value = contact_label(contact->id);
    status.status = "ready";
    return status;
}

DesktopPanelMissileModel make_missile_model(
    const presentation::TacticalMissileTrack& missile) {
    DesktopPanelMissileModel model;
    model.heading = missile_label(missile.id);
    model.metrics.push_back({"Launcher", entity_id_label(missile.launcher)});
    model.metrics.push_back(
        {"Target",
         domain::is_valid(missile.target_contact) ? contact_label(missile.target_contact)
                                                  : std::string("unknown")});
    model.metrics.push_back({"Position", format_position(missile.position_km)});
    model.metrics.push_back({"Velocity", format_velocity(missile.velocity_km_per_second)});
    model.metrics.push_back({"Speed", number(magnitude(missile.velocity_km_per_second)) + " km/s"});
    model.metrics.push_back({"Status", missile_status_label(missile.status)});
    return model;
}

std::string make_hover_summary(const presentation::TacticalSnapshot& snapshot,
                               const DesktopInteractionState& desktop_state) {
    if (desktop_state.hover.kind == DesktopMapObjectKind::FriendlyEntity) {
        const auto* entity = find_friendly(snapshot, desktop_state.hover.entity);
        if (entity == nullptr) {
            return entity_id_label(desktop_state.hover.entity) + " unavailable";
        }
        return entity_label(*entity);
    }

    if (desktop_state.hover.kind == DesktopMapObjectKind::HostileContact) {
        return contact_label(desktop_state.hover.contact);
    }

    if (desktop_state.hover.kind == DesktopMapObjectKind::MissileTrack) {
        for (const auto& missile : snapshot.missile_tracks) {
            if (missile.id == desktop_state.hover.missile) {
                return missile_label(missile.id) + " " + missile_status_label(missile.status);
            }
        }
        return missile_label(desktop_state.hover.missile) + " unavailable";
    }

    return "None";
}

std::vector<std::string> make_event_lines(const presentation::TacticalSnapshot& snapshot) {
    std::vector<std::string> lines;
    const std::size_t first =
        snapshot.events.size() > kEventLogLimit ? snapshot.events.size() - kEventLogLimit : 0;
    lines.reserve(snapshot.events.size() - first);

    for (std::size_t index = first; index < snapshot.events.size(); ++index) {
        const auto& event = snapshot.events[index];
        std::ostringstream stream;
        stream << '[' << event.tick << "] "
               << severity_label(event.severity) << ' '
               << event_type_label(event.type) << " subject="
               << event.subject.value << " - " << event.message;
        lines.push_back(stream.str());
    }

    return lines;
}

} // namespace

DesktopPanelModel make_desktop_panel_model(
    const TacticalUiState& ui_state,
    const DesktopInteractionState& desktop_state,
    const presentation::TacticalSnapshot& snapshot,
    const gameplay::TimeScaleRecommendation& time_scale) {
    DesktopPanelModel model;
    model.time_scale_label = number(time_scale.scale) + "x";
    model.time_scale_reason = gameplay::time_scale_reason_label(time_scale.reason);
    model.hover_summary = make_hover_summary(snapshot, desktop_state);
    model.selection = make_selection_model(snapshot, desktop_state);
    model.staged_statuses.push_back(
        make_staged_launcher_status(snapshot, desktop_state.staged_launcher));
    model.staged_statuses.push_back(
        make_staged_target_status(snapshot, desktop_state.staged_target));

    model.missiles.reserve(snapshot.missile_tracks.size());
    for (const auto& missile : snapshot.missile_tracks) {
        model.missiles.push_back(make_missile_model(missile));
    }

    model.event_lines = make_event_lines(snapshot);
    model.command_log_lines = ui_state.command_log;
    return model;
}

} // namespace scs::ui
