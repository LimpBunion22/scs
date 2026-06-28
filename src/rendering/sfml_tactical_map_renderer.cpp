#include "rendering/sfml_tactical_map_renderer.h"

#include <algorithm>
#include <cmath>

#include "rendering/tactical_map_overlay.h"

namespace scs::rendering {
namespace {

float as_float(double value) {
    return static_cast<float>(value);
}

sf::Vector2f as_vector(ScreenPoint point) {
    return sf::Vector2f{as_float(point.x), as_float(point.y)};
}

sf::FloatRect viewport_rect(TacticalMapProjection projection) {
    projection = normalize_overlay_projection(projection);
    return sf::FloatRect{
        as_float(projection.viewport.left),
        as_float(projection.viewport.top),
        as_float(projection.viewport.width),
        as_float(projection.viewport.height),
    };
}

bool is_selected(TacticalSelection selection, domain::EntityId entity) {
    return selection.kind == TacticalSelectionKind::FriendlyEntity && selection.entity == entity;
}

bool is_selected(TacticalSelection selection, domain::ContactId contact) {
    return selection.kind == TacticalSelectionKind::HostileContact && selection.contact == contact;
}

void draw_circle_marker(sf::RenderTarget& target,
                        ScreenPoint center,
                        float radius,
                        sf::Color fill,
                        sf::Color outline,
                        float outline_thickness) {
    sf::CircleShape marker(radius);
    marker.setOrigin(radius, radius);
    marker.setPosition(as_vector(center));
    marker.setFillColor(fill);
    marker.setOutlineColor(outline);
    marker.setOutlineThickness(outline_thickness);
    target.draw(marker);
}

void draw_grid(sf::RenderTarget& target,
               TacticalMapProjection projection,
               const TacticalMapOverlay& overlay,
               const SfmlTacticalMapStyle& style) {
    projection = normalize_overlay_projection(projection);
    const auto rect = viewport_rect(projection);

    sf::RectangleShape background(sf::Vector2f{rect.width, rect.height});
    background.setPosition(rect.left, rect.top);
    background.setFillColor(style.background);
    target.draw(background);

    const auto draw_world_grid = [&](double spacing_km, sf::Color color) {
        sf::VertexArray lines(sf::Lines);

        const double first_x =
            std::floor(overlay.visible_world_bounds.left_km / spacing_km) * spacing_km;
        for (double x = first_x; x <= overlay.visible_world_bounds.right_km + spacing_km;
             x += spacing_km) {
            const float screen_x = as_float(world_to_screen(
                domain::Vec2{x, projection.center_km.y}, projection).x);
            lines.append(sf::Vertex(sf::Vector2f{screen_x, rect.top}, color));
            lines.append(sf::Vertex(sf::Vector2f{screen_x, rect.top + rect.height}, color));
        }

        const double first_y =
            std::floor(overlay.visible_world_bounds.bottom_km / spacing_km) * spacing_km;
        for (double y = first_y; y <= overlay.visible_world_bounds.top_km + spacing_km;
             y += spacing_km) {
            const float screen_y = as_float(world_to_screen(
                domain::Vec2{projection.center_km.x, y}, projection).y);
            lines.append(sf::Vertex(sf::Vector2f{rect.left, screen_y}, color));
            lines.append(sf::Vertex(sf::Vector2f{rect.left + rect.width, screen_y}, color));
        }

        target.draw(lines);
    };

    draw_world_grid(overlay.minor_grid_spacing_km, style.grid_minor);
    draw_world_grid(overlay.major_grid_spacing_km, style.grid_major);
}

void draw_reference_overlays(sf::RenderTarget& target,
                             TacticalMapProjection projection,
                             const TacticalMapOverlay& overlay,
                             const SfmlTacticalMapStyle& style) {
    projection = normalize_overlay_projection(projection);
    const auto rect = viewport_rect(projection);

    const float left = as_float(projection.viewport.left + 18.0);
    const float bottom = as_float(projection.viewport.top + projection.viewport.height - 18.0);
    const float scale_pixels = as_float(overlay.scale_bar_length_pixels);

    sf::VertexArray scale_bar(sf::Lines, 6);
    scale_bar[0].position = sf::Vector2f{left, bottom};
    scale_bar[1].position = sf::Vector2f{left + scale_pixels, bottom};
    scale_bar[2].position = sf::Vector2f{left, bottom - 6.0F};
    scale_bar[3].position = sf::Vector2f{left, bottom + 6.0F};
    scale_bar[4].position = sf::Vector2f{left + scale_pixels, bottom - 6.0F};
    scale_bar[5].position = sf::Vector2f{left + scale_pixels, bottom + 6.0F};
    for (std::size_t i = 0; i < 6; ++i) {
        scale_bar[i].color = style.border;
    }
    target.draw(scale_bar);

    sf::RectangleShape border(sf::Vector2f{rect.width, rect.height});
    border.setPosition(rect.left, rect.top);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(style.border);
    border.setOutlineThickness(1.0F);
    target.draw(border);
}

void draw_trajectories(sf::RenderTarget& target,
                       const presentation::TacticalSnapshot& snapshot,
                       TacticalMapProjection projection,
                       const SfmlTacticalMapStyle& style) {
    for (const auto& trajectory : snapshot.predicted_trajectories) {
        if (trajectory.points.size() < 2) {
            continue;
        }

        sf::VertexArray line(sf::LineStrip);
        for (const auto& point : trajectory.points) {
            const ScreenPoint screen = world_to_screen(point.position_km, projection);
            line.append(sf::Vertex(as_vector(screen), style.trajectory));
        }
        target.draw(line);
    }
}

void draw_contacts(sf::RenderTarget& target,
                   const presentation::TacticalSnapshot& snapshot,
                   TacticalMapProjection projection,
                   const TacticalMapOverlay& overlay,
                   TacticalSelection selection,
                   TacticalMapHighlight highlight,
                   const SfmlTacticalMapStyle& style) {
    for (const auto& contact : snapshot.hostile_contacts) {
        const ScreenPoint screen = world_to_screen(contact.estimated_position_km, projection);
        if (!is_inside_viewport(screen, projection)) {
            continue;
        }

        const float uncertainty_radius = as_float(
            uncertainty_radius_pixels(overlay, contact.uncertainty_radius_km, projection));
        sf::CircleShape uncertainty(uncertainty_radius);
        uncertainty.setOrigin(uncertainty_radius, uncertainty_radius);
        uncertainty.setPosition(as_vector(screen));
        uncertainty.setFillColor(sf::Color::Transparent);
        uncertainty.setOutlineColor(style.uncertainty);
        uncertainty.setOutlineThickness(1.0F);
        target.draw(uncertainty);

        const float marker_radius = as_float(overlay.marker_radius_pixels);
        sf::CircleShape marker(marker_radius, 4);
        marker.setOrigin(marker_radius, marker_radius);
        marker.setPosition(as_vector(screen));
        marker.setRotation(45.0F);
        marker.setFillColor(style.contact);
        marker.setOutlineColor(is_selected(selection, contact.id) ? style.selection : style.contact);
        marker.setOutlineThickness(is_selected(selection, contact.id) ? 2.5F : 1.0F);
        target.draw(marker);

        if (highlight.contact == contact.id) {
            draw_circle_marker(target,
                               screen,
                               as_float(overlay.hover_ring_radius_pixels),
                               sf::Color::Transparent,
                               style.hover,
                               1.5F);
        }
    }
}

void draw_friendlies(sf::RenderTarget& target,
                     const presentation::TacticalSnapshot& snapshot,
                     TacticalMapProjection projection,
                     const TacticalMapOverlay& overlay,
                     TacticalSelection selection,
                     TacticalMapHighlight highlight,
                     const SfmlTacticalMapStyle& style) {
    for (const auto& entity : snapshot.friendly_entities) {
        const ScreenPoint screen = world_to_screen(entity.position_km, projection);
        if (!is_inside_viewport(screen, projection)) {
            continue;
        }

        draw_circle_marker(target,
                           screen,
                           as_float(overlay.marker_radius_pixels),
                           style.friendly,
                           is_selected(selection, entity.id) ? style.selection : style.friendly,
                           is_selected(selection, entity.id) ? 2.5F : 1.0F);

        if (highlight.entity == entity.id) {
            draw_circle_marker(target,
                               screen,
                               as_float(overlay.hover_ring_radius_pixels),
                               sf::Color::Transparent,
                               style.hover,
                               1.5F);
        }
    }
}

void draw_missiles(sf::RenderTarget& target,
                   const presentation::TacticalSnapshot& snapshot,
                   TacticalMapProjection projection,
                   const TacticalMapOverlay& overlay,
                   TacticalMapHighlight highlight,
                   const SfmlTacticalMapStyle& style) {
    for (const auto& missile : snapshot.missile_tracks) {
        const ScreenPoint screen = world_to_screen(missile.position_km, projection);
        if (!is_inside_viewport(screen, projection)) {
            continue;
        }

        draw_circle_marker(target,
                           screen,
                           as_float(overlay.missile_marker_radius_pixels),
                           style.missile,
                           style.missile,
                           1.0F);
        if (highlight.missile == missile.id) {
            draw_circle_marker(target,
                               screen,
                               as_float(overlay.hover_ring_radius_pixels - 1.5),
                               sf::Color::Transparent,
                               style.hover,
                               1.5F);
        }
    }
}

void draw_edge_hints(sf::RenderTarget& target,
                     const TacticalMapOverlay& overlay,
                     TacticalSelection selection,
                     TacticalMapHighlight highlight,
                     const SfmlTacticalMapStyle& style) {
    for (const auto& hint : overlay.edge_hints) {
        sf::Color color = style.friendly;
        float radius = as_float(overlay.marker_radius_pixels);
        if (hint.kind == TacticalMapOverlayObjectKind::HostileContact) {
            color = style.contact;
        } else if (hint.kind == TacticalMapOverlayObjectKind::MissileTrack) {
            color = style.missile;
            radius = as_float(overlay.missile_marker_radius_pixels);
        }

        const bool selected =
            (hint.kind == TacticalMapOverlayObjectKind::FriendlyEntity &&
             is_selected(selection, hint.entity)) ||
            (hint.kind == TacticalMapOverlayObjectKind::HostileContact &&
             is_selected(selection, hint.contact));
        const bool hovered =
            (hint.kind == TacticalMapOverlayObjectKind::FriendlyEntity &&
             highlight.entity == hint.entity) ||
            (hint.kind == TacticalMapOverlayObjectKind::HostileContact &&
             highlight.contact == hint.contact) ||
            (hint.kind == TacticalMapOverlayObjectKind::MissileTrack &&
             highlight.missile == hint.missile);

        sf::VertexArray stem(sf::Lines, 2);
        stem[0].position = sf::Vector2f{
            as_float(hint.screen_position.x - hint.direction_unit.x * 9.0),
            as_float(hint.screen_position.y - hint.direction_unit.y * 9.0),
        };
        stem[1].position = sf::Vector2f{
            as_float(hint.screen_position.x - hint.direction_unit.x * 2.0),
            as_float(hint.screen_position.y - hint.direction_unit.y * 2.0),
        };
        stem[0].color = color;
        stem[1].color = color;
        target.draw(stem);

        if (hint.kind == TacticalMapOverlayObjectKind::HostileContact) {
            sf::CircleShape marker(radius, 4);
            marker.setOrigin(radius, radius);
            marker.setPosition(as_vector(hint.screen_position));
            marker.setRotation(45.0F);
            marker.setFillColor(color);
            marker.setOutlineColor(selected ? style.selection : color);
            marker.setOutlineThickness(selected ? 2.5F : 1.0F);
            target.draw(marker);
        } else {
            draw_circle_marker(target,
                               hint.screen_position,
                               radius,
                               color,
                               selected ? style.selection : color,
                               selected ? 2.5F : 1.0F);
        }

        if (hovered) {
            draw_circle_marker(target,
                               hint.screen_position,
                               as_float(overlay.hover_ring_radius_pixels),
                               sf::Color::Transparent,
                               style.hover,
                               1.5F);
        }
    }
}

} // namespace

void draw_sfml_tactical_map(sf::RenderTarget& target,
                            const presentation::TacticalSnapshot& snapshot,
                            TacticalMapProjection projection,
                            TacticalSelection selection,
                            TacticalMapHighlight highlight,
                            const SfmlTacticalMapStyle& style) {
    projection = normalize_overlay_projection(projection);
    const TacticalMapOverlay overlay = make_tactical_map_overlay(snapshot, projection);

    draw_grid(target, projection, overlay, style);
    draw_trajectories(target, snapshot, projection, style);
    draw_missiles(target, snapshot, projection, overlay, highlight, style);
    draw_contacts(target, snapshot, projection, overlay, selection, highlight, style);
    draw_friendlies(target, snapshot, projection, overlay, selection, highlight, style);
    draw_edge_hints(target, overlay, selection, highlight, style);
    draw_reference_overlays(target, projection, overlay, style);
}

} // namespace scs::rendering
