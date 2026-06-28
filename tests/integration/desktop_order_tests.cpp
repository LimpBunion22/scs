#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "presentation/tactical_snapshot.h"
#include "ui/desktop_interaction.h"
#include "ui/desktop_order_model.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

scs::presentation::TacticalSnapshot make_snapshot() {
    scs::presentation::TacticalSnapshot snapshot;
    snapshot.tick = 25;
    snapshot.time_seconds = 25.0;
    snapshot.friendly_entities.push_back(scs::domain::EntitySnapshot{
        scs::domain::EntityId{1},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Friendly,
        "Blue One",
        scs::domain::Vec2{0.0, 0.0},
        scs::domain::Vec2{0.0, 0.0},
        2,
        1,
    });
    snapshot.friendly_entities.push_back(scs::domain::EntitySnapshot{
        scs::domain::EntityId{2},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Friendly,
        "Blue Two",
        scs::domain::Vec2{10.0, 0.0},
        scs::domain::Vec2{1.0, 0.0},
        1,
        0,
    });
    snapshot.hostile_contacts.push_back(scs::domain::ContactSnapshot{
        scs::domain::ContactId{9},
        scs::domain::EntityId{1},
        scs::domain::Vec2{80.0, 0.0},
        scs::domain::Vec2{-1.0, 0.0},
        25,
        0.9,
        scs::domain::ContactClassification::HostileCombatGroup,
        5.0,
    });
    return snapshot;
}

void velocity_order_emits_for_selected_visible_friendly() {
    const auto snapshot = make_snapshot();
    scs::ui::DesktopInteractionState state;
    state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::FriendlyEntity,
        scs::domain::EntityId{2},
        scs::domain::ContactId{},
    };
    state.staged_velocity_x_km_per_second = 12.5;
    state.staged_velocity_y_km_per_second = -3.0;

    const auto result = scs::ui::emit_desktop_velocity_command(state, snapshot);

    require(result.command.has_value(), "Velocity order did not emit a command.");
    require(result.command->execute_on == snapshot.tick,
            "Velocity order used the wrong execution tick.");
    const auto* payload =
        std::get_if<scs::domain::SetVelocityCommand>(&result.command->payload);
    require(payload != nullptr, "Velocity order emitted the wrong payload type.");
    require(payload->target == scs::domain::EntityId{2},
            "Velocity order used the wrong selected friendly.");
    require(payload->velocity_km_per_second.x == 12.5,
            "Velocity order used the wrong x velocity.");
    require(payload->velocity_km_per_second.y == -3.0,
            "Velocity order used the wrong y velocity.");
}

void velocity_order_emits_for_staged_visible_friendly() {
    const auto snapshot = make_snapshot();
    scs::ui::DesktopInteractionState state;
    state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::HostileContact,
        scs::domain::EntityId{},
        scs::domain::ContactId{9},
    };
    state.staged_launcher = scs::domain::EntityId{1};
    state.staged_velocity_x_km_per_second = 4.0;
    state.staged_velocity_y_km_per_second = 6.0;

    const auto result = scs::ui::emit_desktop_velocity_command(state, snapshot);

    require(result.command.has_value(),
            "Velocity order did not use the staged visible friendly.");
    const auto* payload =
        std::get_if<scs::domain::SetVelocityCommand>(&result.command->payload);
    require(payload != nullptr, "Velocity order emitted the wrong payload type.");
    require(payload->target == scs::domain::EntityId{1},
            "Velocity order used the wrong staged friendly.");
}

void velocity_order_rejects_when_no_friendly_is_selected_or_staged() {
    const auto snapshot = make_snapshot();
    scs::ui::DesktopInteractionState state;
    state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::HostileContact,
        scs::domain::EntityId{},
        scs::domain::ContactId{9},
    };

    const auto result = scs::ui::emit_desktop_velocity_command(state, snapshot);

    require(!result.command.has_value(),
            "Velocity order emitted without a selected or staged friendly.");
    require(result.feedback.find("Select or stage a visible friendly") != std::string::npos,
            "Velocity order did not explain the missing friendly rejection.");
}

void velocity_order_rejects_hidden_staged_friendly() {
    const auto snapshot = make_snapshot();
    scs::ui::DesktopInteractionState state;
    state.staged_launcher = scs::domain::EntityId{99};

    const auto result = scs::ui::emit_desktop_velocity_command(state, snapshot);

    require(!result.command.has_value(),
            "Velocity order emitted for a staged friendly absent from the snapshot.");
    require(result.feedback.find("not visible") != std::string::npos,
            "Velocity order did not explain the hidden friendly rejection.");
}

void velocity_order_rejects_hidden_selected_friendly() {
    const auto snapshot = make_snapshot();
    scs::ui::DesktopInteractionState state;
    state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::FriendlyEntity,
        scs::domain::EntityId{99},
        scs::domain::ContactId{},
    };

    const auto result = scs::ui::emit_desktop_velocity_command(state, snapshot);

    require(!result.command.has_value(),
            "Velocity order emitted for a selected friendly absent from the snapshot.");
    require(result.feedback.find("Selected friendly is not visible") != std::string::npos,
            "Velocity order did not explain the hidden selected friendly rejection.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"velocity_order_emits_for_selected_visible_friendly",
         velocity_order_emits_for_selected_visible_friendly},
        {"velocity_order_emits_for_staged_visible_friendly",
         velocity_order_emits_for_staged_visible_friendly},
        {"velocity_order_rejects_when_no_friendly_is_selected_or_staged",
         velocity_order_rejects_when_no_friendly_is_selected_or_staged},
        {"velocity_order_rejects_hidden_staged_friendly",
         velocity_order_rejects_hidden_staged_friendly},
        {"velocity_order_rejects_hidden_selected_friendly",
         velocity_order_rejects_hidden_selected_friendly},
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
