#include "ui/imgui_tactical_panels.h"

#include <algorithm>
#include <string>

#include <imgui.h>

namespace scs::ui {
namespace {

constexpr std::size_t kEventLogLimit = 10;

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

const presentation::TacticalMissileTrack* find_missile(
    const presentation::TacticalSnapshot& snapshot,
    domain::MissileId id) {
    for (const auto& missile : snapshot.missile_tracks) {
        if (missile.id == id) {
            return &missile;
        }
    }
    return nullptr;
}

void draw_selection(const presentation::TacticalSnapshot& snapshot,
                    const DesktopInteractionState& desktop_state) {
    ImGui::SeparatorText("Selection");
    if (desktop_state.selection.kind == rendering::TacticalSelectionKind::FriendlyEntity) {
        const auto* entity = find_friendly(snapshot, desktop_state.selection.entity);
        if (entity == nullptr) {
            ImGui::TextUnformatted("Friendly unavailable");
            return;
        }

        ImGui::Text("F%u %s", entity->id.value, entity->name.c_str());
        ImGui::Text("Position %.1f, %.1f km", entity->position_km.x, entity->position_km.y);
        ImGui::Text("Velocity %.1f, %.1f km/s",
                    entity->velocity_km_per_second.x,
                    entity->velocity_km_per_second.y);
        ImGui::Text("Missiles %d  Defenses %d",
                    entity->missile_ammunition,
                    entity->defensive_response_charges);
        return;
    }

    if (desktop_state.selection.kind == rendering::TacticalSelectionKind::HostileContact) {
        const auto* contact = find_contact(snapshot, desktop_state.selection.contact);
        if (contact == nullptr) {
            ImGui::TextUnformatted("Contact unavailable");
            return;
        }

        ImGui::Text("C%u observed by F%u", contact->id.value, contact->observer.value);
        ImGui::Text("Estimate %.1f, %.1f km",
                    contact->estimated_position_km.x,
                    contact->estimated_position_km.y);
        ImGui::Text("Velocity %.1f, %.1f km/s",
                    contact->estimated_velocity_km_per_second.x,
                    contact->estimated_velocity_km_per_second.y);
        ImGui::Text("Confidence %.2f  Uncertainty %.1f km",
                    contact->confidence,
                    contact->uncertainty_radius_km);
        return;
    }

    ImGui::TextUnformatted("None");
}

void draw_hover(const presentation::TacticalSnapshot& snapshot,
                const DesktopInteractionState& desktop_state) {
    ImGui::SeparatorText("Hover");
    if (desktop_state.hover.kind == DesktopMapObjectKind::FriendlyEntity) {
        const auto* entity = find_friendly(snapshot, desktop_state.hover.entity);
        ImGui::Text("F%u %s",
                    desktop_state.hover.entity.value,
                    entity == nullptr ? "" : entity->name.c_str());
        return;
    }
    if (desktop_state.hover.kind == DesktopMapObjectKind::HostileContact) {
        ImGui::Text("C%u", desktop_state.hover.contact.value);
        return;
    }
    if (desktop_state.hover.kind == DesktopMapObjectKind::MissileTrack) {
        const auto* missile = find_missile(snapshot, desktop_state.hover.missile);
        ImGui::Text("M%u %s",
                    desktop_state.hover.missile.value,
                    missile == nullptr ? "" : missile_status_label(missile->status));
        return;
    }
    ImGui::TextUnformatted("None");
}

void draw_events(const presentation::TacticalSnapshot& snapshot) {
    ImGui::SeparatorText("Events");
    if (snapshot.events.empty()) {
        ImGui::TextUnformatted("none");
        return;
    }

    const std::size_t first =
        snapshot.events.size() > kEventLogLimit ? snapshot.events.size() - kEventLogLimit : 0;
    for (std::size_t i = first; i < snapshot.events.size(); ++i) {
        const auto& event = snapshot.events[i];
        ImGui::TextWrapped("[%llu] %s %s subject=%u - %s",
                           static_cast<unsigned long long>(event.tick),
                           severity_label(event.severity),
                           event_type_label(event.type),
                           event.subject.value,
                           event.message.c_str());
    }
}

void draw_command_log(const TacticalUiState& ui_state) {
    ImGui::SeparatorText("Command Log");
    if (ui_state.command_log.empty()) {
        ImGui::TextUnformatted("none");
        return;
    }

    for (const auto& entry : ui_state.command_log) {
        ImGui::TextWrapped("%s", entry.c_str());
    }
}

void draw_staged_engagement(const DesktopInteractionState& desktop_state) {
    ImGui::Text("Launcher: %s",
                domain::is_valid(desktop_state.staged_launcher)
                    ? ("F" + std::to_string(desktop_state.staged_launcher.value)).c_str()
                    : "none");
    ImGui::Text("Target: %s",
                domain::is_valid(desktop_state.staged_target)
                    ? ("C" + std::to_string(desktop_state.staged_target.value)).c_str()
                    : "none");
}

} // namespace

DesktopCommandResult draw_imgui_tactical_panels(
    TacticalUiState& ui_state,
    DesktopInteractionState& desktop_state,
    const presentation::TacticalSnapshot& snapshot,
    const gameplay::TimeScaleRecommendation& time_scale,
    std::string_view scenario_name,
    ImguiPanelLayout layout) {
    DesktopCommandResult result;

    ImGui::SetNextWindowPos(ImVec2{layout.left, layout.top}, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2{layout.width, layout.height}, ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Tactical Command", nullptr, flags);

    const std::string scenario{scenario_name};
    ImGui::TextWrapped("%s", scenario.c_str());
    ImGui::Text("Tick %llu  Time %.1fs",
                static_cast<unsigned long long>(snapshot.tick),
                snapshot.time_seconds);
    ImGui::Text("Scale %.1fx  %s",
                time_scale.scale,
                gameplay::time_scale_reason_label(time_scale.reason));

    ImGui::SeparatorText("Time");
    if (ImGui::Button(ui_state.tactical_pause ? "Resume" : "Pause")) {
        ui_state.tactical_pause = !ui_state.tactical_pause;
        result.feedback = ui_state.tactical_pause ? "Tactical pause enabled."
                                                  : "Tactical pause cleared.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        result.advance_ticks = 1;
        result.feedback = "Simulation step requested.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Run")) {
        if (ui_state.tactical_pause) {
            result.feedback = "Simulation is paused; resume or step explicitly.";
        } else {
            result.advance_ticks = 1;
            result.feedback = "Simulation run tick requested.";
        }
    }

    if (ImGui::Button("Auto Scale")) {
        ui_state.manual_time_scale.reset();
        result.feedback = "Automatic time scale restored.";
    }
    ImGui::SameLine();
    if (ImGui::Button("1x")) {
        ui_state.manual_time_scale = 1.0;
        result.feedback = "Manual time scale override set.";
    }
    ImGui::SameLine();
    if (ImGui::Button("8x")) {
        ui_state.manual_time_scale = 8.0;
        result.feedback = "Manual time scale override set.";
    }
    ImGui::SameLine();
    if (ImGui::Button("64x")) {
        ui_state.manual_time_scale = 64.0;
        result.feedback = "Manual time scale override set.";
    }

    draw_selection(snapshot, desktop_state);
    draw_hover(snapshot, desktop_state);

    ImGui::SeparatorText("Engagement");
    draw_staged_engagement(desktop_state);
    if (ImGui::Button("Engage Contact")) {
        result = emit_staged_desktop_engage_contact(desktop_state, snapshot);
    }

    draw_events(snapshot);
    draw_command_log(ui_state);

    if (ImGui::Button("Quit")) {
        result.quit = true;
        result.feedback = "Quit requested.";
        result.advance_ticks = 0;
    }

    ImGui::End();
    return result;
}

} // namespace scs::ui
