#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_overlay.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool close(double lhs, double rhs, double epsilon = 1e-6) {
    return std::abs(lhs - rhs) <= epsilon;
}

scs::presentation::TacticalSnapshot make_snapshot_fixture() {
    scs::presentation::TacticalSnapshot snapshot;
    snapshot.tick = 25;
    snapshot.time_seconds = 25.0;
    snapshot.friendly_entities.push_back(scs::domain::EntitySnapshot{
        scs::domain::EntityId{1},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Friendly,
        "Blue Center",
        scs::domain::Vec2{0.0, 0.0},
        scs::domain::Vec2{0.0, 0.0},
        1,
        0,
    });
    snapshot.friendly_entities.push_back(scs::domain::EntitySnapshot{
        scs::domain::EntityId{2},
        scs::domain::EntityKind::CombatGroup,
        scs::domain::Allegiance::Friendly,
        "Blue Wing",
        scs::domain::Vec2{-600.0, 0.0},
        scs::domain::Vec2{0.0, 0.0},
        1,
        0,
    });
    snapshot.hostile_contacts.push_back(scs::domain::ContactSnapshot{
        scs::domain::ContactId{9},
        scs::domain::EntityId{1},
        scs::domain::Vec2{0.0, 500.0},
        scs::domain::Vec2{0.0, -1.0},
        25,
        0.8,
        scs::domain::ContactClassification::HostileCombatGroup,
        4.0,
    });
    snapshot.missile_tracks.push_back(scs::presentation::TacticalMissileTrack{
        scs::domain::MissileId{4},
        scs::domain::EntityId{1},
        scs::domain::ContactId{9},
        scs::domain::Vec2{600.0, -500.0},
        scs::domain::Vec2{-2.0, 1.0},
        scs::domain::MissileStatus::InFlight,
    });
    return snapshot;
}

scs::rendering::TacticalMapProjection make_projection(double kilometers_per_pixel) {
    return scs::rendering::TacticalMapProjection{
        scs::domain::Vec2{0.0, 0.0},
        kilometers_per_pixel,
        scs::rendering::ScreenRect{0.0, 0.0, 800.0, 600.0},
    };
}

const scs::rendering::TacticalMapEdgeHint* find_entity_hint(
    const scs::rendering::TacticalMapOverlay& overlay,
    scs::domain::EntityId entity) {
    for (const auto& hint : overlay.edge_hints) {
        if (hint.kind == scs::rendering::TacticalMapOverlayObjectKind::FriendlyEntity &&
            hint.entity == entity) {
            return &hint;
        }
    }
    return nullptr;
}

const scs::rendering::TacticalMapEdgeHint* find_contact_hint(
    const scs::rendering::TacticalMapOverlay& overlay,
    scs::domain::ContactId contact) {
    for (const auto& hint : overlay.edge_hints) {
        if (hint.kind == scs::rendering::TacticalMapOverlayObjectKind::HostileContact &&
            hint.contact == contact) {
            return &hint;
        }
    }
    return nullptr;
}

const scs::rendering::TacticalMapEdgeHint* find_missile_hint(
    const scs::rendering::TacticalMapOverlay& overlay,
    scs::domain::MissileId missile) {
    for (const auto& hint : overlay.edge_hints) {
        if (hint.kind == scs::rendering::TacticalMapOverlayObjectKind::MissileTrack &&
            hint.missile == missile) {
            return &hint;
        }
    }
    return nullptr;
}

void overlay_tracks_visible_bounds_and_scale_across_zoom_levels() {
    const auto snapshot = make_snapshot_fixture();
    const auto close_projection = make_projection(0.25);
    const auto medium_projection = make_projection(1.0);
    const auto far_projection = make_projection(250.0);

    const auto close_overlay =
        scs::rendering::make_tactical_map_overlay(snapshot, close_projection);
    const auto medium_overlay =
        scs::rendering::make_tactical_map_overlay(snapshot, medium_projection);
    const auto far_overlay = scs::rendering::make_tactical_map_overlay(snapshot, far_projection);

    require(close(close_overlay.visible_world_bounds.left_km, -100.0),
            "Close zoom left world bound is wrong.");
    require(close(close_overlay.visible_world_bounds.right_km, 100.0),
            "Close zoom right world bound is wrong.");
    require(close(close_overlay.visible_world_bounds.bottom_km, -75.0),
            "Close zoom bottom world bound is wrong.");
    require(close(close_overlay.visible_world_bounds.top_km, 75.0),
            "Close zoom top world bound is wrong.");

    require(close_overlay.minor_grid_spacing_km < medium_overlay.minor_grid_spacing_km,
            "Grid spacing should increase as zoom moves out.");
    require(medium_overlay.minor_grid_spacing_km < far_overlay.minor_grid_spacing_km,
            "Grid spacing should keep increasing at far zoom.");
    require(close_overlay.scale_bar_length_km < medium_overlay.scale_bar_length_km,
            "Scale bar kilometers should increase as zoom moves out.");
    require(medium_overlay.scale_bar_length_km < far_overlay.scale_bar_length_km,
            "Scale bar kilometers should keep increasing at far zoom.");

    require(close_overlay.minor_grid_spacing_pixels >= 64.0 &&
                close_overlay.minor_grid_spacing_pixels <= 128.0,
            "Close zoom grid spacing pixels fell outside the readable band.");
    require(medium_overlay.minor_grid_spacing_pixels >= 64.0 &&
                medium_overlay.minor_grid_spacing_pixels <= 128.0,
            "Medium zoom grid spacing pixels fell outside the readable band.");
    require(far_overlay.minor_grid_spacing_pixels >= 64.0 &&
                far_overlay.minor_grid_spacing_pixels <= 128.0,
            "Far zoom grid spacing pixels fell outside the readable band.");

    require(close_overlay.scale_bar_length_pixels >= 96.0 &&
                close_overlay.scale_bar_length_pixels <= 180.0,
            "Close zoom scale bar pixels fell outside the readable band.");
    require(medium_overlay.scale_bar_length_pixels >= 96.0 &&
                medium_overlay.scale_bar_length_pixels <= 180.0,
            "Medium zoom scale bar pixels fell outside the readable band.");
    require(far_overlay.scale_bar_length_pixels >= 96.0 &&
                far_overlay.scale_bar_length_pixels <= 180.0,
            "Far zoom scale bar pixels fell outside the readable band.");

    require(close_overlay.marker_radius_pixels <= medium_overlay.marker_radius_pixels,
            "Marker radius should not shrink when zooming out.");
    require(medium_overlay.marker_radius_pixels <= far_overlay.marker_radius_pixels,
            "Marker radius should keep pace at far zoom.");
}

void overlay_keeps_contact_uncertainty_and_missiles_visible_at_default_zoom() {
    const auto snapshot = make_snapshot_fixture();
    const auto projection = make_projection(1.0);
    const auto overlay = scs::rendering::make_tactical_map_overlay(snapshot, projection);

    const double contact_radius = scs::rendering::uncertainty_radius_pixels(
        overlay, snapshot.hostile_contacts.front().uncertainty_radius_km, projection);

    require(contact_radius >= overlay.minimum_uncertainty_radius_pixels,
            "Small uncertainty radius did not clamp to a visible minimum.");
    require(overlay.missile_marker_radius_pixels >= 3.0,
            "Missile marker radius is too small for the default scenario zoom.");
    require(overlay.marker_radius_pixels >= 5.0,
            "Friendly/contact markers are too small for the default scenario zoom.");
}

void overlay_places_offscreen_hints_on_view_edges() {
    const auto snapshot = make_snapshot_fixture();
    const auto projection = make_projection(1.0);
    const auto overlay = scs::rendering::make_tactical_map_overlay(snapshot, projection);
    const double inset = overlay.hover_ring_radius_pixels + 4.0;

    require(overlay.edge_hints.size() == 3,
            "Expected one edge hint each for the off-screen friendly, contact, and missile.");

    const auto* entity_hint = find_entity_hint(overlay, scs::domain::EntityId{2});
    require(entity_hint != nullptr, "Missing edge hint for off-screen friendly entity.");
    require(close(entity_hint->screen_position.x, inset),
            "Off-screen left friendly hint did not land on the left edge inset.");
    require(entity_hint->direction_unit.x < 0.0,
            "Off-screen left friendly hint points the wrong way.");

    const auto* contact_hint = find_contact_hint(overlay, scs::domain::ContactId{9});
    require(contact_hint != nullptr, "Missing edge hint for off-screen hostile contact.");
    require(close(contact_hint->screen_position.y, inset),
            "Off-screen top contact hint did not land on the top edge inset.");
    require(contact_hint->direction_unit.y < 0.0,
            "Off-screen top contact hint points the wrong way.");

    const auto* missile_hint = find_missile_hint(overlay, scs::domain::MissileId{4});
    require(missile_hint != nullptr, "Missing edge hint for off-screen missile.");
    require(close(missile_hint->screen_position.x, 800.0 - inset) ||
                close(missile_hint->screen_position.y, 600.0 - inset),
            "Off-screen missile hint did not land on the viewport edge inset.");
    require(missile_hint->direction_unit.x > 0.0 && missile_hint->direction_unit.y > 0.0,
            "Off-screen missile hint points the wrong way.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"overlay_tracks_visible_bounds_and_scale_across_zoom_levels",
         overlay_tracks_visible_bounds_and_scale_across_zoom_levels},
        {"overlay_keeps_contact_uncertainty_and_missiles_visible_at_default_zoom",
         overlay_keeps_contact_uncertainty_and_missiles_visible_at_default_zoom},
        {"overlay_places_offscreen_hints_on_view_edges",
         overlay_places_offscreen_hints_on_view_edges},
    };

    for (const auto& test : tests) {
        try {
            test.second();
        } catch (const std::exception& error) {
            std::cerr << test.first << " failed: " << error.what() << '\n';
            return 1;
        }
    }

    std::cout << "desktop render model tests passed\n";
    return 0;
}
