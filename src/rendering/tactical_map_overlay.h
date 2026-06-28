#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

#include "domain/contact.h"
#include "domain/entity.h"
#include "domain/ids.h"
#include "domain/vector2.h"
#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_projection.h"

namespace scs::rendering {

struct TacticalMapWorldBounds {
    double left_km{0.0};
    double right_km{0.0};
    double bottom_km{0.0};
    double top_km{0.0};
};

enum class TacticalMapOverlayObjectKind {
    FriendlyEntity,
    HostileContact,
    MissileTrack
};

struct TacticalMapEdgeHint {
    TacticalMapOverlayObjectKind kind{TacticalMapOverlayObjectKind::FriendlyEntity};
    domain::EntityId entity;
    domain::ContactId contact;
    domain::MissileId missile;
    ScreenPoint screen_position;
    ScreenPoint direction_unit;
};

struct TacticalMapOverlay {
    TacticalMapWorldBounds visible_world_bounds;
    double minor_grid_spacing_km{1.0};
    double major_grid_spacing_km{5.0};
    double minor_grid_spacing_pixels{1.0};
    double major_grid_spacing_pixels{5.0};
    double scale_bar_length_km{1.0};
    double scale_bar_length_pixels{1.0};
    double marker_radius_pixels{6.0};
    double missile_marker_radius_pixels{4.0};
    double hover_ring_radius_pixels{11.0};
    double minimum_uncertainty_radius_pixels{12.0};
    std::vector<TacticalMapEdgeHint> edge_hints;
};

[[nodiscard]] inline TacticalMapProjection normalize_overlay_projection(
    TacticalMapProjection projection) {
    return normalized_projection(projection);
}

[[nodiscard]] inline bool is_inside_viewport(ScreenPoint point,
                                             TacticalMapProjection projection) {
    projection = normalize_overlay_projection(projection);
    return point.x >= projection.viewport.left &&
           point.x <= projection.viewport.left + projection.viewport.width &&
           point.y >= projection.viewport.top &&
           point.y <= projection.viewport.top + projection.viewport.height;
}

[[nodiscard]] inline TacticalMapWorldBounds visible_world_bounds(
    TacticalMapProjection projection) {
    projection = normalize_overlay_projection(projection);
    const double half_width_km = projection.viewport.width * 0.5 * projection.kilometers_per_pixel;
    const double half_height_km =
        projection.viewport.height * 0.5 * projection.kilometers_per_pixel;
    return TacticalMapWorldBounds{
        projection.center_km.x - half_width_km,
        projection.center_km.x + half_width_km,
        projection.center_km.y - half_height_km,
        projection.center_km.y + half_height_km,
    };
}

namespace detail {

[[nodiscard]] inline double choose_nice_distance(double target_km) {
    const double safe_target = std::max(1e-6, target_km);
    const double magnitude = std::pow(10.0, std::floor(std::log10(safe_target)));
    const double normalized = safe_target / magnitude;

    double nice = 10.0;
    if (normalized <= 1.0) {
        nice = 1.0;
    } else if (normalized <= 2.0) {
        nice = 2.0;
    } else if (normalized <= 5.0) {
        nice = 5.0;
    }

    return nice * magnitude;
}

[[nodiscard]] inline double choose_nice_distance_in_band(double target_km,
                                                         double minimum_km,
                                                         double maximum_km) {
    const double safe_target = std::max(1e-6, target_km);
    const double safe_minimum = std::max(1e-6, minimum_km);
    const double safe_maximum = std::max(safe_minimum, maximum_km);

    double best = choose_nice_distance(safe_target);
    double best_score = std::numeric_limits<double>::infinity();
    const double exponent = std::floor(std::log10(safe_target));

    for (int offset = -2; offset <= 2; ++offset) {
        const double magnitude = std::pow(10.0, exponent + static_cast<double>(offset));
        for (const double factor : {1.0, 2.0, 5.0, 10.0}) {
            const double candidate = factor * magnitude;
            if (candidate < safe_minimum || candidate > safe_maximum) {
                continue;
            }

            const double score = std::abs(candidate - safe_target);
            if (score < best_score) {
                best = candidate;
                best_score = score;
            }
        }
    }

    if (best_score < std::numeric_limits<double>::infinity()) {
        return best;
    }

    return std::clamp(best, safe_minimum, safe_maximum);
}

[[nodiscard]] inline double marker_radius_for_projection(
    TacticalMapProjection projection) {
    projection = normalize_overlay_projection(projection);
    const double visible_span_km =
        std::max(projection.viewport.width, projection.viewport.height) *
        projection.kilometers_per_pixel;
    const double scale = std::log10(std::max(1.0, visible_span_km));
    return std::clamp(4.5 + scale * 0.5, 5.0, 9.0);
}

[[nodiscard]] inline std::optional<TacticalMapEdgeHint> make_edge_hint(
    TacticalMapOverlayObjectKind kind,
    domain::EntityId entity,
    domain::ContactId contact,
    domain::MissileId missile,
    domain::Vec2 world_position_km,
    TacticalMapProjection projection,
    double padding_pixels) {
    projection = normalize_overlay_projection(projection);
    const ScreenPoint screen = world_to_screen(world_position_km, projection);
    if (is_inside_viewport(screen, projection)) {
        return std::nullopt;
    }

    const double center_x = projection.viewport.left + projection.viewport.width * 0.5;
    const double center_y = projection.viewport.top + projection.viewport.height * 0.5;
    const double dx = screen.x - center_x;
    const double dy = screen.y - center_y;
    const double length = std::hypot(dx, dy);
    if (length <= 0.0) {
        return std::nullopt;
    }

    const double half_width = std::max(1.0, projection.viewport.width * 0.5 - padding_pixels);
    const double half_height = std::max(1.0, projection.viewport.height * 0.5 - padding_pixels);
    const double tx =
        dx == 0.0 ? std::numeric_limits<double>::infinity() : half_width / std::abs(dx);
    const double ty =
        dy == 0.0 ? std::numeric_limits<double>::infinity() : half_height / std::abs(dy);
    const double t = std::min(tx, ty);

    TacticalMapEdgeHint hint;
    hint.kind = kind;
    hint.entity = entity;
    hint.contact = contact;
    hint.missile = missile;
    hint.screen_position = ScreenPoint{center_x + dx * t, center_y + dy * t};
    hint.direction_unit = ScreenPoint{dx / length, dy / length};
    return hint;
}

} // namespace detail

[[nodiscard]] inline double uncertainty_radius_pixels(
    const TacticalMapOverlay& overlay,
    double uncertainty_radius_km,
    TacticalMapProjection projection) {
    projection = normalize_overlay_projection(projection);
    return std::max(overlay.minimum_uncertainty_radius_pixels,
                    uncertainty_radius_km / projection.kilometers_per_pixel);
}

[[nodiscard]] inline TacticalMapOverlay make_tactical_map_overlay(
    const presentation::TacticalSnapshot& snapshot,
    TacticalMapProjection projection) {
    projection = normalize_overlay_projection(projection);

    TacticalMapOverlay overlay;
    overlay.visible_world_bounds = visible_world_bounds(projection);
    overlay.marker_radius_pixels = detail::marker_radius_for_projection(projection);
    overlay.missile_marker_radius_pixels =
        std::max(3.0, overlay.marker_radius_pixels * 0.6);
    overlay.hover_ring_radius_pixels = overlay.marker_radius_pixels + 5.0;
    overlay.minimum_uncertainty_radius_pixels =
        std::max(overlay.marker_radius_pixels * 2.0, 12.0);

    const double target_minor_grid_pixels =
        std::clamp(projection.viewport.width * 0.1, 64.0, 128.0);
    overlay.minor_grid_spacing_km =
        detail::choose_nice_distance(target_minor_grid_pixels * projection.kilometers_per_pixel);
    overlay.major_grid_spacing_km = overlay.minor_grid_spacing_km * 5.0;
    overlay.minor_grid_spacing_pixels =
        overlay.minor_grid_spacing_km / projection.kilometers_per_pixel;
    overlay.major_grid_spacing_pixels =
        overlay.major_grid_spacing_km / projection.kilometers_per_pixel;

    const double target_scale_bar_pixels =
        std::clamp(projection.viewport.width * 0.18, 96.0, 180.0);
    overlay.scale_bar_length_km = detail::choose_nice_distance_in_band(
        target_scale_bar_pixels * projection.kilometers_per_pixel,
        96.0 * projection.kilometers_per_pixel,
        180.0 * projection.kilometers_per_pixel);
    overlay.scale_bar_length_pixels =
        overlay.scale_bar_length_km / projection.kilometers_per_pixel;

    const double edge_padding = overlay.hover_ring_radius_pixels + 4.0;
    for (const auto& entity : snapshot.friendly_entities) {
        const auto hint = detail::make_edge_hint(TacticalMapOverlayObjectKind::FriendlyEntity,
                                                 entity.id,
                                                 domain::ContactId{},
                                                 domain::MissileId{},
                                                 entity.position_km,
                                                 projection,
                                                 edge_padding);
        if (hint.has_value()) {
            overlay.edge_hints.push_back(*hint);
        }
    }

    for (const auto& contact : snapshot.hostile_contacts) {
        const auto hint = detail::make_edge_hint(TacticalMapOverlayObjectKind::HostileContact,
                                                 domain::EntityId{},
                                                 contact.id,
                                                 domain::MissileId{},
                                                 contact.estimated_position_km,
                                                 projection,
                                                 edge_padding);
        if (hint.has_value()) {
            overlay.edge_hints.push_back(*hint);
        }
    }

    for (const auto& missile : snapshot.missile_tracks) {
        const auto hint = detail::make_edge_hint(TacticalMapOverlayObjectKind::MissileTrack,
                                                 domain::EntityId{},
                                                 domain::ContactId{},
                                                 missile.id,
                                                 missile.position_km,
                                                 projection,
                                                 edge_padding);
        if (hint.has_value()) {
            overlay.edge_hints.push_back(*hint);
        }
    }

    return overlay;
}

} // namespace scs::rendering
