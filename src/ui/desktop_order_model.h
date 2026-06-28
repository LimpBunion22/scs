#pragma once

#include <algorithm>
#include <optional>
#include <string>

#include "domain/command.h"
#include "presentation/tactical_snapshot.h"
#include "ui/desktop_interaction.h"

namespace scs::ui {

enum class DesktopVelocityOrderReadiness {
    Ready,
    MissingFriendly,
    HiddenFriendly,
};

struct DesktopVelocityOrderModel {
    DesktopVelocityOrderReadiness readiness{DesktopVelocityOrderReadiness::MissingFriendly};
    domain::EntityId target;
    std::string target_label;
    std::string feedback;
};

namespace detail {

inline const domain::EntitySnapshot* find_visible_friendly(
    const presentation::TacticalSnapshot& snapshot,
    domain::EntityId id) {
    const auto it = std::find_if(snapshot.friendly_entities.begin(),
                                 snapshot.friendly_entities.end(),
                                 [id](const auto& entity) { return entity.id == id; });
    if (it == snapshot.friendly_entities.end()) {
        return nullptr;
    }
    return &(*it);
}

inline std::string entity_id_label(domain::EntityId id) {
    return "F" + std::to_string(id.value);
}

} // namespace detail

inline DesktopVelocityOrderModel make_desktop_velocity_order_model(
    const DesktopInteractionState& state,
    const presentation::TacticalSnapshot& snapshot) {
    DesktopVelocityOrderModel model;

    if (state.selection.kind == rendering::TacticalSelectionKind::FriendlyEntity) {
        if (const auto* entity = detail::find_visible_friendly(snapshot, state.selection.entity)) {
            model.readiness = DesktopVelocityOrderReadiness::Ready;
            model.target = entity->id;
            model.target_label = detail::entity_id_label(entity->id) + " " + entity->name;
            model.feedback = "Ready.";
            return model;
        }
    }

    if (domain::is_valid(state.staged_launcher)) {
        if (const auto* entity = detail::find_visible_friendly(snapshot, state.staged_launcher)) {
            model.readiness = DesktopVelocityOrderReadiness::Ready;
            model.target = entity->id;
            model.target_label = detail::entity_id_label(entity->id) + " " + entity->name;
            model.feedback = "Ready.";
            return model;
        }

        model.readiness = DesktopVelocityOrderReadiness::HiddenFriendly;
        model.target = state.staged_launcher;
        model.target_label = detail::entity_id_label(state.staged_launcher);
        model.feedback = "Staged friendly is not visible in the current tactical snapshot.";
        return model;
    }

    if (state.selection.kind == rendering::TacticalSelectionKind::FriendlyEntity &&
        domain::is_valid(state.selection.entity)) {
        model.readiness = DesktopVelocityOrderReadiness::HiddenFriendly;
        model.target = state.selection.entity;
        model.target_label = detail::entity_id_label(state.selection.entity);
        model.feedback = "Selected friendly is not visible in the current tactical snapshot.";
        return model;
    }

    model.readiness = DesktopVelocityOrderReadiness::MissingFriendly;
    model.feedback = "Select or stage a visible friendly entity before issuing velocity.";
    return model;
}

inline DesktopCommandResult emit_desktop_velocity_command(
    const DesktopInteractionState& state,
    const presentation::TacticalSnapshot& snapshot) {
    const DesktopVelocityOrderModel model = make_desktop_velocity_order_model(state, snapshot);
    if (model.readiness != DesktopVelocityOrderReadiness::Ready) {
        return DesktopCommandResult{false, 0, std::nullopt, model.feedback};
    }

    return DesktopCommandResult{
        false,
        0,
        domain::set_velocity_at(snapshot.tick,
                                model.target,
                                domain::Vec2{state.staged_velocity_x_km_per_second,
                                             state.staged_velocity_y_km_per_second}),
        "Velocity command emitted.",
    };
}

} // namespace scs::ui
