#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "domain/contact.h"
#include "domain/command.h"
#include "domain/ids.h"
#include "domain/snapshot.h"
#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_projection.h"
#include "rendering/tactical_map_renderer.h"

namespace scs::ui {

enum class DesktopMapObjectKind {
    None,
    FriendlyEntity,
    HostileContact,
    MissileTrack
};

struct DesktopMapObject {
    DesktopMapObjectKind kind{DesktopMapObjectKind::None};
    domain::EntityId entity;
    domain::ContactId contact;
    domain::MissileId missile;
};

constexpr bool operator==(DesktopMapObject lhs, DesktopMapObject rhs) {
    return lhs.kind == rhs.kind && lhs.entity == rhs.entity &&
           lhs.contact == rhs.contact && lhs.missile == rhs.missile;
}

constexpr bool operator!=(DesktopMapObject lhs, DesktopMapObject rhs) {
    return !(lhs == rhs);
}

struct DesktopHitCandidate {
    DesktopMapObject object;
    double distance_pixels{0.0};
};

struct DesktopHitTestConfig {
    double hover_radius_pixels{16.0};
    double selection_radius_pixels{16.0};
    double cycle_click_radius_pixels{3.0};
};

struct DesktopInteractionState {
    rendering::TacticalSelection selection;
    DesktopMapObject hover;
    domain::EntityId staged_launcher;
    domain::ContactId staged_target;
    double staged_velocity_x_km_per_second{0.0};
    double staged_velocity_y_km_per_second{0.0};
    rendering::ScreenPoint last_click_screen;
    std::vector<DesktopMapObject> cycle_candidates;
    std::size_t cycle_index{0};
    bool has_cycle_anchor{false};
};

struct DesktopCommandResult {
    bool quit{false};
    domain::Tick advance_ticks{0};
    std::optional<domain::Command> command;
    std::string feedback;
};

[[nodiscard]] std::vector<DesktopHitCandidate> desktop_hover_candidates(
    const presentation::TacticalSnapshot& snapshot,
    rendering::TacticalMapProjection projection,
    rendering::ScreenPoint screen,
    const DesktopHitTestConfig& config = DesktopHitTestConfig{});

[[nodiscard]] std::vector<DesktopHitCandidate> desktop_selection_candidates(
    const presentation::TacticalSnapshot& snapshot,
    rendering::TacticalMapProjection projection,
    rendering::ScreenPoint screen,
    const DesktopHitTestConfig& config = DesktopHitTestConfig{});

[[nodiscard]] DesktopMapObject hover_desktop_map_object(
    const presentation::TacticalSnapshot& snapshot,
    rendering::TacticalMapProjection projection,
    rendering::ScreenPoint screen,
    const DesktopHitTestConfig& config = DesktopHitTestConfig{});

[[nodiscard]] rendering::TacticalSelection selection_from_desktop_object(DesktopMapObject object);

void update_desktop_hover(DesktopInteractionState& state,
                          const presentation::TacticalSnapshot& snapshot,
                          rendering::TacticalMapProjection projection,
                          rendering::ScreenPoint screen,
                          const DesktopHitTestConfig& config = DesktopHitTestConfig{});

void select_desktop_map_object(DesktopInteractionState& state,
                               const presentation::TacticalSnapshot& snapshot,
                               rendering::TacticalMapProjection projection,
                               rendering::ScreenPoint screen,
                               const DesktopHitTestConfig& config = DesktopHitTestConfig{});

[[nodiscard]] DesktopCommandResult emit_staged_desktop_engage_contact(
    const DesktopInteractionState& state,
    const presentation::TacticalSnapshot& snapshot);

} // namespace scs::ui
