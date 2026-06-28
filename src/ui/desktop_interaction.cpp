#include "ui/desktop_interaction.h"

#include <algorithm>
#include <cmath>

namespace scs::ui {
namespace {

double distance_squared(rendering::ScreenPoint lhs, rendering::ScreenPoint rhs) {
    const double dx = lhs.x - rhs.x;
    const double dy = lhs.y - rhs.y;
    return dx * dx + dy * dy;
}

double distance(rendering::ScreenPoint lhs, rendering::ScreenPoint rhs) {
    return std::sqrt(distance_squared(lhs, rhs));
}

bool is_selectable(DesktopMapObjectKind kind) {
    return kind == DesktopMapObjectKind::FriendlyEntity ||
           kind == DesktopMapObjectKind::HostileContact;
}

int object_kind_order(DesktopMapObjectKind kind) {
    switch (kind) {
    case DesktopMapObjectKind::FriendlyEntity:
        return 0;
    case DesktopMapObjectKind::HostileContact:
        return 1;
    case DesktopMapObjectKind::MissileTrack:
        return 2;
    case DesktopMapObjectKind::None:
        return 3;
    }
    return 3;
}

std::uint32_t object_id_value(DesktopMapObject object) {
    switch (object.kind) {
    case DesktopMapObjectKind::FriendlyEntity:
        return object.entity.value;
    case DesktopMapObjectKind::HostileContact:
        return object.contact.value;
    case DesktopMapObjectKind::MissileTrack:
        return object.missile.value;
    case DesktopMapObjectKind::None:
        return 0;
    }
    return 0;
}

bool same_cycle_candidates(const std::vector<DesktopMapObject>& lhs,
                           const std::vector<DesktopHitCandidate>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rhs[i].object) {
            return false;
        }
    }
    return true;
}

bool has_friendly_entity(const presentation::TacticalSnapshot& snapshot, domain::EntityId id) {
    return std::any_of(snapshot.friendly_entities.begin(),
                       snapshot.friendly_entities.end(),
                       [id](const auto& entity) { return entity.id == id; });
}

bool has_hostile_contact(const presentation::TacticalSnapshot& snapshot, domain::ContactId id) {
    return std::any_of(snapshot.hostile_contacts.begin(),
                       snapshot.hostile_contacts.end(),
                       [id](const auto& contact) { return contact.id == id; });
}

std::vector<DesktopHitCandidate> make_candidates(
    const presentation::TacticalSnapshot& snapshot,
    rendering::TacticalMapProjection projection,
    rendering::ScreenPoint screen,
    double radius_pixels,
    bool selectable_only) {
    std::vector<DesktopHitCandidate> candidates;

    const auto append_candidate = [&](DesktopMapObject object, domain::Vec2 position_km) {
        if (selectable_only && !is_selectable(object.kind)) {
            return;
        }

        const double candidate_distance =
            distance(rendering::world_to_screen(position_km, projection), screen);
        if (candidate_distance <= radius_pixels) {
            candidates.push_back(DesktopHitCandidate{object, candidate_distance});
        }
    };

    for (const auto& entity : snapshot.friendly_entities) {
        append_candidate(DesktopMapObject{
                             DesktopMapObjectKind::FriendlyEntity,
                             entity.id,
                             domain::ContactId{},
                             domain::MissileId{},
                         },
                         entity.position_km);
    }

    for (const auto& contact : snapshot.hostile_contacts) {
        append_candidate(DesktopMapObject{
                             DesktopMapObjectKind::HostileContact,
                             domain::EntityId{},
                             contact.id,
                             domain::MissileId{},
                         },
                         contact.estimated_position_km);
    }

    for (const auto& missile : snapshot.missile_tracks) {
        append_candidate(DesktopMapObject{
                             DesktopMapObjectKind::MissileTrack,
                             domain::EntityId{},
                             domain::ContactId{},
                             missile.id,
                         },
                         missile.position_km);
    }

    std::sort(candidates.begin(), candidates.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.distance_pixels != rhs.distance_pixels) {
            return lhs.distance_pixels < rhs.distance_pixels;
        }
        const int lhs_kind = object_kind_order(lhs.object.kind);
        const int rhs_kind = object_kind_order(rhs.object.kind);
        if (lhs_kind != rhs_kind) {
            return lhs_kind < rhs_kind;
        }
        return object_id_value(lhs.object) < object_id_value(rhs.object);
    });

    return candidates;
}

} // namespace

std::vector<DesktopHitCandidate> desktop_hover_candidates(
    const presentation::TacticalSnapshot& snapshot,
    rendering::TacticalMapProjection projection,
    rendering::ScreenPoint screen,
    const DesktopHitTestConfig& config) {
    return make_candidates(snapshot, projection, screen, config.hover_radius_pixels, false);
}

std::vector<DesktopHitCandidate> desktop_selection_candidates(
    const presentation::TacticalSnapshot& snapshot,
    rendering::TacticalMapProjection projection,
    rendering::ScreenPoint screen,
    const DesktopHitTestConfig& config) {
    return make_candidates(snapshot, projection, screen, config.selection_radius_pixels, true);
}

DesktopMapObject hover_desktop_map_object(const presentation::TacticalSnapshot& snapshot,
                                          rendering::TacticalMapProjection projection,
                                          rendering::ScreenPoint screen,
                                          const DesktopHitTestConfig& config) {
    const auto candidates = desktop_hover_candidates(snapshot, projection, screen, config);
    if (candidates.empty()) {
        return DesktopMapObject{};
    }
    return candidates.front().object;
}

rendering::TacticalSelection selection_from_desktop_object(DesktopMapObject object) {
    if (object.kind == DesktopMapObjectKind::FriendlyEntity) {
        return rendering::TacticalSelection{
            rendering::TacticalSelectionKind::FriendlyEntity,
            object.entity,
            domain::ContactId{},
        };
    }
    if (object.kind == DesktopMapObjectKind::HostileContact) {
        return rendering::TacticalSelection{
            rendering::TacticalSelectionKind::HostileContact,
            domain::EntityId{},
            object.contact,
        };
    }
    return rendering::TacticalSelection{};
}

void update_desktop_hover(DesktopInteractionState& state,
                          const presentation::TacticalSnapshot& snapshot,
                          rendering::TacticalMapProjection projection,
                          rendering::ScreenPoint screen,
                          const DesktopHitTestConfig& config) {
    state.hover = hover_desktop_map_object(snapshot, projection, screen, config);
}

void select_desktop_map_object(DesktopInteractionState& state,
                               const presentation::TacticalSnapshot& snapshot,
                               rendering::TacticalMapProjection projection,
                               rendering::ScreenPoint screen,
                               const DesktopHitTestConfig& config) {
    const auto candidates = desktop_selection_candidates(snapshot, projection, screen, config);
    if (candidates.empty()) {
        state.selection = rendering::TacticalSelection{};
        state.cycle_candidates.clear();
        state.cycle_index = 0;
        state.has_cycle_anchor = false;
        return;
    }

    const bool can_continue_cycle =
        state.has_cycle_anchor &&
        distance(state.last_click_screen, screen) <= config.cycle_click_radius_pixels &&
        same_cycle_candidates(state.cycle_candidates, candidates);

    if (can_continue_cycle) {
        state.cycle_index = (state.cycle_index + 1) % candidates.size();
    } else {
        state.cycle_candidates.clear();
        for (const auto& candidate : candidates) {
            state.cycle_candidates.push_back(candidate.object);
        }
        state.cycle_index = 0;
        state.has_cycle_anchor = true;
    }

    state.last_click_screen = screen;
    const DesktopMapObject selected = candidates[state.cycle_index].object;
    state.selection = selection_from_desktop_object(selected);
    if (selected.kind == DesktopMapObjectKind::FriendlyEntity) {
        state.staged_launcher = selected.entity;
    }
    if (selected.kind == DesktopMapObjectKind::HostileContact) {
        state.staged_target = selected.contact;
    }
}

DesktopCommandResult emit_staged_desktop_engage_contact(
    const DesktopInteractionState& state,
    const presentation::TacticalSnapshot& snapshot) {
    if (!domain::is_valid(state.staged_launcher) ||
        !has_friendly_entity(snapshot, state.staged_launcher)) {
        return DesktopCommandResult{false, 0, std::nullopt, "Select a visible friendly launcher."};
    }

    if (!domain::is_valid(state.staged_target) ||
        !has_hostile_contact(snapshot, state.staged_target)) {
        return DesktopCommandResult{false, 0, std::nullopt, "Select a visible hostile contact."};
    }

    return DesktopCommandResult{
        false,
        0,
        domain::engage_contact_at(snapshot.tick, state.staged_launcher, state.staged_target),
        "Engage contact command emitted.",
    };
}

} // namespace scs::ui
