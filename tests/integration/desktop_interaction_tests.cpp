#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_projection.h"
#include "ui/desktop_interaction.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool close(double lhs, double rhs) {
    return std::abs(lhs - rhs) < 1e-9;
}

scs::presentation::TacticalSnapshot make_desktop_fixture() {
    scs::presentation::TacticalSnapshot snapshot;
    snapshot.tick = 10;
    snapshot.time_seconds = 10.0;
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
        scs::domain::Vec2{2.0, 0.0},
        scs::domain::Vec2{0.0, 0.0},
        1,
        0,
    });
    snapshot.hostile_contacts.push_back(scs::domain::ContactSnapshot{
        scs::domain::ContactId{7},
        scs::domain::EntityId{1},
        scs::domain::Vec2{30.0, 0.0},
        scs::domain::Vec2{0.0, 0.0},
        10,
        1.0,
        scs::domain::ContactClassification::HostileCombatGroup,
        4.0,
    });
    snapshot.missile_tracks.push_back(scs::presentation::TacticalMissileTrack{
        scs::domain::MissileId{3},
        scs::domain::EntityId{1},
        scs::domain::ContactId{7},
        scs::domain::Vec2{-20.0, 0.0},
        scs::domain::Vec2{1.0, 0.0},
        scs::domain::MissileStatus::InFlight,
    });
    return snapshot;
}

scs::rendering::TacticalMapProjection make_projection() {
    return scs::rendering::TacticalMapProjection{
        scs::domain::Vec2{0.0, 0.0},
        1.0,
        scs::rendering::ScreenRect{0.0, 0.0, 800.0, 600.0},
    };
}

void projection_round_trips_world_and_screen_points() {
    const auto projection = make_projection();
    const scs::domain::Vec2 world{50.0, -25.0};
    const auto screen = scs::rendering::world_to_screen(world, projection);
    const auto round_trip = scs::rendering::screen_to_world(screen, projection);

    require(close(round_trip.x, world.x), "Projection did not round-trip x.");
    require(close(round_trip.y, world.y), "Projection did not round-trip y.");
}

void pan_and_zoom_keep_cursor_anchor_stable() {
    auto projection = make_projection();
    scs::rendering::pan_projection_by_pixels(projection, scs::rendering::ScreenPoint{10.0, -5.0});
    require(close(projection.center_km.x, -10.0), "Pan did not move center in x.");
    require(close(projection.center_km.y, -5.0), "Pan did not move center in y.");

    const scs::rendering::ScreenPoint anchor{500.0, 325.0};
    const auto before = scs::rendering::screen_to_world(anchor, projection);
    scs::rendering::zoom_projection_around_screen_point(projection, anchor, 0.5);
    const auto after = scs::rendering::screen_to_world(anchor, projection);

    require(close(after.x, before.x), "Zoom did not preserve anchored world x.");
    require(close(after.y, before.y), "Zoom did not preserve anchored world y.");
    require(close(projection.kilometers_per_pixel, 0.5), "Zoom did not change scale.");
}

void hit_test_selects_nearest_visible_friendly_or_contact() {
    const auto snapshot = make_desktop_fixture();
    const auto projection = make_projection();
    scs::ui::DesktopInteractionState state;

    scs::ui::select_desktop_map_object(
        state,
        snapshot,
        projection,
        scs::rendering::world_to_screen(scs::domain::Vec2{30.0, 0.0}, projection));

    require(state.selection.kind == scs::rendering::TacticalSelectionKind::HostileContact,
            "Click near contact did not select a contact.");
    require(state.selection.contact == scs::domain::ContactId{7},
            "Click selected the wrong contact.");
}

void hit_test_cycles_overlapping_selectable_objects() {
    const auto snapshot = make_desktop_fixture();
    const auto projection = make_projection();
    scs::ui::DesktopInteractionState state;
    const auto click = scs::rendering::world_to_screen(scs::domain::Vec2{1.0, 0.0}, projection);

    scs::ui::select_desktop_map_object(state, snapshot, projection, click);
    require(state.selection.kind == scs::rendering::TacticalSelectionKind::FriendlyEntity,
            "First overlap click did not select a friendly.");
    require(state.selection.entity == scs::domain::EntityId{1},
            "First overlap click selected the wrong friendly.");

    scs::ui::select_desktop_map_object(state, snapshot, projection, click);
    require(state.selection.kind == scs::rendering::TacticalSelectionKind::FriendlyEntity,
            "Second overlap click did not cycle to a friendly.");
    require(state.selection.entity == scs::domain::EntityId{2},
            "Second overlap click did not cycle to the next friendly.");
}

void selecting_friendly_and_contact_stages_engagement_command() {
    const auto snapshot = make_desktop_fixture();
    const auto projection = make_projection();
    scs::ui::DesktopInteractionState state;

    scs::ui::select_desktop_map_object(
        state,
        snapshot,
        projection,
        scs::rendering::world_to_screen(scs::domain::Vec2{0.0, 0.0}, projection));
    scs::ui::select_desktop_map_object(
        state,
        snapshot,
        projection,
        scs::rendering::world_to_screen(scs::domain::Vec2{30.0, 0.0}, projection));

    const auto result = scs::ui::emit_staged_desktop_engage_contact(state, snapshot);

    require(result.command.has_value(), "Staged engage did not emit a command.");
    require(result.command->execute_on == snapshot.tick,
            "Staged engage command used the wrong tick.");
    const auto* payload =
        std::get_if<scs::domain::EngageContactCommand>(&result.command->payload);
    require(payload != nullptr, "Staged engage emitted the wrong command payload.");
    require(payload->launcher == scs::domain::EntityId{1},
            "Staged engage used the wrong launcher.");
    require(payload->target == scs::domain::ContactId{7},
            "Staged engage used the wrong target contact.");
}

void staged_engage_rejects_missing_visible_target() {
    const auto snapshot = make_desktop_fixture();
    scs::ui::DesktopInteractionState state;
    state.staged_launcher = scs::domain::EntityId{1};

    const auto result = scs::ui::emit_staged_desktop_engage_contact(state, snapshot);

    require(!result.command.has_value(),
            "Staged engage emitted a command without a visible contact.");
    require(result.feedback.find("hostile contact") != std::string::npos,
            "Staged engage did not explain the missing contact.");
}

void missiles_hover_but_do_not_become_selection() {
    const auto snapshot = make_desktop_fixture();
    const auto projection = make_projection();
    scs::ui::DesktopInteractionState state;
    const auto missile_screen =
        scs::rendering::world_to_screen(scs::domain::Vec2{-20.0, 0.0}, projection);

    scs::ui::update_desktop_hover(state, snapshot, projection, missile_screen);
    require(state.hover.kind == scs::ui::DesktopMapObjectKind::MissileTrack,
            "Missile was not available as a hover target.");
    require(state.hover.missile == scs::domain::MissileId{3},
            "Hovered the wrong missile.");

    scs::ui::select_desktop_map_object(state, snapshot, projection, missile_screen);
    require(state.selection.kind == scs::rendering::TacticalSelectionKind::None,
            "Missile click should not create a selection.");
}

void hit_test_does_not_return_objects_absent_from_snapshot() {
    const auto snapshot = make_desktop_fixture();
    const auto projection = make_projection();
    const auto hidden_contact_screen =
        scs::rendering::world_to_screen(scs::domain::Vec2{100.0, 0.0}, projection);

    const auto hover =
        scs::ui::hover_desktop_map_object(snapshot, projection, hidden_contact_screen);
    const auto candidates =
        scs::ui::desktop_selection_candidates(snapshot, projection, hidden_contact_screen);

    require(hover.kind == scs::ui::DesktopMapObjectKind::None,
            "Hit test exposed an object not present in the tactical snapshot.");
    require(candidates.empty(),
            "Selection candidates exposed an object not present in the tactical snapshot.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"projection_round_trips_world_and_screen_points",
         projection_round_trips_world_and_screen_points},
        {"pan_and_zoom_keep_cursor_anchor_stable", pan_and_zoom_keep_cursor_anchor_stable},
        {"hit_test_selects_nearest_visible_friendly_or_contact",
         hit_test_selects_nearest_visible_friendly_or_contact},
        {"hit_test_cycles_overlapping_selectable_objects",
         hit_test_cycles_overlapping_selectable_objects},
        {"selecting_friendly_and_contact_stages_engagement_command",
         selecting_friendly_and_contact_stages_engagement_command},
        {"staged_engage_rejects_missing_visible_target",
         staged_engage_rejects_missing_visible_target},
        {"missiles_hover_but_do_not_become_selection", missiles_hover_but_do_not_become_selection},
        {"hit_test_does_not_return_objects_absent_from_snapshot",
         hit_test_does_not_return_objects_absent_from_snapshot},
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
