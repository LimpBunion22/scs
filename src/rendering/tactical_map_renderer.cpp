#include "rendering/tactical_map_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace scs::rendering {
namespace {

constexpr int kMinimumMapCells = 5;
constexpr std::size_t kEventLogLimit = 8;
constexpr double kPi = 3.14159265358979323846;

TacticalMapView normalized_view(TacticalMapView view) {
    if (view.width_cells < kMinimumMapCells) {
        view.width_cells = kMinimumMapCells;
    }
    if (view.height_cells < kMinimumMapCells) {
        view.height_cells = kMinimumMapCells;
    }
    if (view.kilometers_per_cell <= 0.0) {
        view.kilometers_per_cell = 100'000.0;
    }
    return view;
}

std::string number(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    return stream.str();
}

double dot(domain::Vec2 lhs, domain::Vec2 rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

double magnitude(domain::Vec2 value) {
    return std::sqrt(domain::magnitude_squared(value));
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

bool is_selected(const TacticalSelection& selection, domain::EntityId entity) {
    return selection.kind == TacticalSelectionKind::FriendlyEntity && selection.entity == entity;
}

bool is_selected(const TacticalSelection& selection, domain::ContactId contact) {
    return selection.kind == TacticalSelectionKind::HostileContact && selection.contact == contact;
}

bool project(domain::Vec2 position,
             const TacticalMapView& view,
             int& row,
             int& column) {
    const int center_column = view.width_cells / 2;
    const int center_row = view.height_cells / 2;
    column = center_column +
             static_cast<int>(std::llround((position.x - view.center_km.x) /
                                           view.kilometers_per_cell));
    row = center_row -
          static_cast<int>(std::llround((position.y - view.center_km.y) /
                                        view.kilometers_per_cell));

    return row >= 0 && row < view.height_cells &&
           column >= 0 && column < view.width_cells;
}

void mark(std::vector<std::string>& grid,
          const TacticalMapView& view,
          domain::Vec2 position,
          char marker,
          bool overwrite = true) {
    int row = 0;
    int column = 0;
    if (!project(position, view, row, column)) {
        return;
    }

    if (overwrite || grid[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] == ' ') {
        grid[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] = marker;
    }
}

const domain::EntitySnapshot* find_friendly_entity(const presentation::TacticalSnapshot& snapshot,
                                                   domain::EntityId id) {
    for (const auto& entity : snapshot.friendly_entities) {
        if (entity.id == id) {
            return &entity;
        }
    }
    return nullptr;
}

const domain::ContactSnapshot* find_hostile_contact(const presentation::TacticalSnapshot& snapshot,
                                                    domain::ContactId id) {
    for (const auto& contact : snapshot.hostile_contacts) {
        if (contact.id == id) {
            return &contact;
        }
    }
    return nullptr;
}

void render_trajectories(std::vector<std::string>& grid,
                         const TacticalMapView& view,
                         const presentation::TacticalSnapshot& snapshot) {
    for (const auto& trajectory : snapshot.predicted_trajectories) {
        bool first_point = true;
        for (const auto& point : trajectory.points) {
            if (!first_point) {
                mark(grid, view, point.position_km, '.', false);
            }
            first_point = false;
        }
    }
}

void render_contacts(std::vector<std::string>& grid,
                     const TacticalMapView& view,
                     const presentation::TacticalSnapshot& snapshot,
                     const TacticalSelection& selection) {
    for (const auto& contact : snapshot.hostile_contacts) {
        mark(grid,
             view,
             contact.estimated_position_km,
             is_selected(selection, contact.id) ? '*' : 'C');
    }
}

void render_friendlies(std::vector<std::string>& grid,
                       const TacticalMapView& view,
                       const presentation::TacticalSnapshot& snapshot,
                       const TacticalSelection& selection) {
    for (const auto& entity : snapshot.friendly_entities) {
        mark(grid,
             view,
             entity.position_km,
             is_selected(selection, entity.id) ? '*' : 'F');
    }
}

void render_missiles(std::vector<std::string>& grid,
                     const TacticalMapView& view,
                     const presentation::TacticalSnapshot& snapshot) {
    for (const auto& missile : snapshot.missile_tracks) {
        mark(grid, view, missile.position_km, 'M');
    }
}

void append_entity_list(std::ostringstream& stream,
                        const presentation::TacticalSnapshot& snapshot,
                        const TacticalSelection& selection) {
    stream << "Friendly groups\n";
    if (snapshot.friendly_entities.empty()) {
        stream << "  none\n";
        return;
    }

    for (const auto& entity : snapshot.friendly_entities) {
        stream << "  " << (is_selected(selection, entity.id) ? '*' : ' ')
               << " F" << entity.id.value << ' ' << entity.name
               << " pos=(" << number(entity.position_km.x) << ", "
               << number(entity.position_km.y) << ") km"
               << " vel=(" << number(entity.velocity_km_per_second.x) << ", "
               << number(entity.velocity_km_per_second.y) << ") km/s"
               << " missiles=" << entity.missile_ammunition
               << " defenses=" << entity.defensive_response_charges << '\n';
    }
}

void append_contact_list(std::ostringstream& stream,
                         const presentation::TacticalSnapshot& snapshot,
                         const TacticalSelection& selection) {
    stream << "Hostile contacts\n";
    if (snapshot.hostile_contacts.empty()) {
        stream << "  none\n";
        return;
    }

    for (const auto& contact : snapshot.hostile_contacts) {
        stream << "  " << (is_selected(selection, contact.id) ? '*' : ' ')
               << " C" << contact.id.value
               << " observer=F" << contact.observer.value
               << " pos=(" << number(contact.estimated_position_km.x) << ", "
               << number(contact.estimated_position_km.y) << ") km"
               << " vel=(" << number(contact.estimated_velocity_km_per_second.x) << ", "
               << number(contact.estimated_velocity_km_per_second.y) << ") km/s"
               << " confidence=" << number(contact.confidence)
               << " uncertainty_km=" << number(contact.uncertainty_radius_km)
               << " last_tick=" << contact.last_observed_tick << '\n';
    }
}

void append_missile_list(std::ostringstream& stream,
                         const presentation::TacticalSnapshot& snapshot) {
    stream << "Missile tracks\n";
    if (snapshot.missile_tracks.empty()) {
        stream << "  none\n";
        return;
    }

    for (const auto& missile : snapshot.missile_tracks) {
        stream << "  M" << missile.id.value
               << " launcher=F" << missile.launcher.value
               << " target_contact=";
        if (domain::is_valid(missile.target_contact)) {
            stream << 'C' << missile.target_contact.value;
        } else {
            stream << "unknown";
        }
        stream << " pos=(" << number(missile.position_km.x) << ", "
               << number(missile.position_km.y) << ") km"
               << " vel=(" << number(missile.velocity_km_per_second.x) << ", "
               << number(missile.velocity_km_per_second.y) << ") km/s"
               << " speed=" << number(magnitude(missile.velocity_km_per_second)) << " km/s"
               << " status=" << missile_status_label(missile.status) << '\n';
    }
}

void append_selected_friendly(std::ostringstream& stream,
                              const domain::EntitySnapshot& entity) {
    stream << "Selected friendly\n";
    stream << "  id=F" << entity.id.value << " name=" << entity.name << '\n';
    stream << "  position=(" << number(entity.position_km.x) << ", "
           << number(entity.position_km.y) << ") km\n";
    stream << "  velocity=(" << number(entity.velocity_km_per_second.x) << ", "
           << number(entity.velocity_km_per_second.y) << ") km/s"
           << " speed=" << number(magnitude(entity.velocity_km_per_second)) << " km/s\n";
    stream << "  missiles=" << entity.missile_ammunition
           << " defenses=" << entity.defensive_response_charges << '\n';
}

void append_contact_observer_metrics(std::ostringstream& stream,
                                     const domain::ContactSnapshot& contact,
                                     const domain::EntitySnapshot* observer) {
    if (observer == nullptr) {
        stream << "  observer_metrics=unknown\n";
        return;
    }

    const domain::Vec2 relative_position = contact.estimated_position_km - observer->position_km;
    const domain::Vec2 relative_velocity =
        contact.estimated_velocity_km_per_second - observer->velocity_km_per_second;
    const double range_km = magnitude(relative_position);
    const double relative_speed_squared = domain::magnitude_squared(relative_velocity);
    const double closest_time_seconds =
        relative_speed_squared == 0.0
            ? 0.0
            : std::max(0.0, -dot(relative_position, relative_velocity) / relative_speed_squared);
    const domain::Vec2 closest_offset =
        relative_position + relative_velocity * closest_time_seconds;

    stream << "  observer_metrics range=" << number(range_km) << " km";
    if (range_km == 0.0) {
        stream << " bearing=unknown";
    } else {
        stream << " bearing=" << number(bearing_degrees(relative_position)) << " deg";
    }
    stream << " closing_speed=";
    if (range_km == 0.0) {
        stream << "unknown";
    } else {
        stream << number(-dot(relative_position, relative_velocity) / range_km) << " km/s";
    }
    stream << " closest_approach_time=" << number(closest_time_seconds) << " s"
           << " closest_approach_distance=" << number(magnitude(closest_offset)) << " km\n";
}

void append_selected_contact(std::ostringstream& stream,
                             const presentation::TacticalSnapshot& snapshot,
                             const domain::ContactSnapshot& contact) {
    stream << "Selected contact\n";
    stream << "  id=C" << contact.id.value << " observer=F" << contact.observer.value << '\n';
    stream << "  estimated_position=(" << number(contact.estimated_position_km.x) << ", "
           << number(contact.estimated_position_km.y) << ") km\n";
    stream << "  estimated_velocity=(" << number(contact.estimated_velocity_km_per_second.x) << ", "
           << number(contact.estimated_velocity_km_per_second.y) << ") km/s\n";
    stream << "  confidence=" << number(contact.confidence)
           << " uncertainty=" << number(contact.uncertainty_radius_km) << " km";
    if (snapshot.tick >= contact.last_observed_tick) {
        stream << " age=" << (snapshot.tick - contact.last_observed_tick) << " ticks\n";
    } else {
        stream << " age=unknown\n";
    }
    append_contact_observer_metrics(stream, contact, find_friendly_entity(snapshot, contact.observer));
}

void append_selection_metrics(std::ostringstream& stream,
                              const presentation::TacticalSnapshot& snapshot,
                              const TacticalSelection& selection) {
    if (selection.kind == TacticalSelectionKind::None) {
        stream << "Selection\n  none\n";
        return;
    }

    if (selection.kind == TacticalSelectionKind::FriendlyEntity) {
        const auto* entity = find_friendly_entity(snapshot, selection.entity);
        if (entity == nullptr) {
            stream << "Selected friendly\n  F" << selection.entity.value << " unavailable\n";
            return;
        }
        append_selected_friendly(stream, *entity);
        return;
    }

    const auto* contact = find_hostile_contact(snapshot, selection.contact);
    if (contact == nullptr) {
        stream << "Selected contact\n  C" << selection.contact.value << " unavailable\n";
        return;
    }
    append_selected_contact(stream, snapshot, *contact);
}

void append_event_log(std::ostringstream& stream,
                      const presentation::TacticalSnapshot& snapshot) {
    stream << "Events\n";
    if (snapshot.events.empty()) {
        stream << "  none\n";
        return;
    }

    const std::size_t first =
        snapshot.events.size() > kEventLogLimit ? snapshot.events.size() - kEventLogLimit : 0;
    for (std::size_t i = first; i < snapshot.events.size(); ++i) {
        const auto& event = snapshot.events[i];
        stream << "  [" << event.tick << "] "
               << severity_label(event.severity) << ' '
               << event_type_label(event.type) << " subject="
               << event.subject.value << " - " << event.message << '\n';
    }
}

} // namespace

std::string render_tactical_map(const presentation::TacticalSnapshot& snapshot,
                                TacticalMapView view,
                                TacticalSelection selection,
                                std::optional<gameplay::TimeScaleRecommendation> time_scale) {
    view = normalized_view(view);

    std::vector<std::string> grid(static_cast<std::size_t>(view.height_cells),
                                  std::string(static_cast<std::size_t>(view.width_cells), ' '));
    render_trajectories(grid, view, snapshot);
    render_missiles(grid, view, snapshot);
    render_contacts(grid, view, snapshot, selection);
    render_friendlies(grid, view, snapshot, selection);

    std::ostringstream stream;
    stream << "Tactical map tick=" << snapshot.tick
           << " time=" << number(snapshot.time_seconds) << "s\n";
    stream << "Reference center=(" << number(view.center_km.x) << ", "
           << number(view.center_km.y) << ") km"
           << " kilometers_per_cell=" << number(view.kilometers_per_cell)
           << " visible_span=(" << number(view.kilometers_per_cell *
                                          static_cast<double>(view.width_cells))
           << " x " << number(view.kilometers_per_cell *
                               static_cast<double>(view.height_cells)) << ") km\n";
    stream << "Orientation: +x east, +y north\n";
    if (time_scale.has_value()) {
        stream << "Time scale=" << number(time_scale->scale) << "x"
               << " reason=" << gameplay::time_scale_reason_label(time_scale->reason) << '\n';
    }
    stream << "Legend: F friendly, C contact, M missile, . prediction, * selected\n";

    const std::string border(static_cast<std::size_t>(view.width_cells), '-');
    stream << '+' << border << "+\n";
    for (const auto& row : grid) {
        stream << '|' << row << "|\n";
    }
    stream << '+' << border << "+\n";

    append_entity_list(stream, snapshot, selection);
    append_contact_list(stream, snapshot, selection);
    append_missile_list(stream, snapshot);
    append_selection_metrics(stream, snapshot, selection);
    append_event_log(stream, snapshot);

    return stream.str();
}

} // namespace scs::rendering
