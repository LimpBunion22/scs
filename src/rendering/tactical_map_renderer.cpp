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
                                TacticalSelection selection) {
    view = normalized_view(view);

    std::vector<std::string> grid(static_cast<std::size_t>(view.height_cells),
                                  std::string(static_cast<std::size_t>(view.width_cells), ' '));
    render_trajectories(grid, view, snapshot);
    render_contacts(grid, view, snapshot, selection);
    render_friendlies(grid, view, snapshot, selection);

    std::ostringstream stream;
    stream << "Tactical map tick=" << snapshot.tick
           << " time=" << number(snapshot.time_seconds) << "s\n";
    stream << "View center=(" << number(view.center_km.x) << ", "
           << number(view.center_km.y) << ") km"
           << " scale=" << number(view.kilometers_per_cell) << " km/cell\n";
    stream << "Legend: F friendly, C contact, . prediction, * selected\n";

    const std::string border(static_cast<std::size_t>(view.width_cells), '-');
    stream << '+' << border << "+\n";
    for (const auto& row : grid) {
        stream << '|' << row << "|\n";
    }
    stream << '+' << border << "+\n";

    append_entity_list(stream, snapshot, selection);
    append_contact_list(stream, snapshot, selection);
    append_event_log(stream, snapshot);

    return stream.str();
}

} // namespace scs::rendering
