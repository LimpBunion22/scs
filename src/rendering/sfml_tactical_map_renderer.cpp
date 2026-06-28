#include "rendering/sfml_tactical_map_renderer.h"

#include <algorithm>
#include <cmath>

namespace scs::rendering {
namespace {

float as_float(double value) {
    return static_cast<float>(value);
}

sf::Vector2f as_vector(ScreenPoint point) {
    return sf::Vector2f{as_float(point.x), as_float(point.y)};
}

sf::FloatRect viewport_rect(TacticalMapProjection projection) {
    projection = normalized_projection(projection);
    return sf::FloatRect{
        as_float(projection.viewport.left),
        as_float(projection.viewport.top),
        as_float(projection.viewport.width),
        as_float(projection.viewport.height),
    };
}

bool is_inside(ScreenPoint point, TacticalMapProjection projection) {
    projection = normalized_projection(projection);
    return point.x >= projection.viewport.left &&
           point.x <= projection.viewport.left + projection.viewport.width &&
           point.y >= projection.viewport.top &&
           point.y <= projection.viewport.top + projection.viewport.height;
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
               const SfmlTacticalMapStyle& style) {
    projection = normalized_projection(projection);
    const auto rect = viewport_rect(projection);

    sf::RectangleShape background(sf::Vector2f{rect.width, rect.height});
    background.setPosition(rect.left, rect.top);
    background.setFillColor(style.background);
    target.draw(background);

    const double minor_spacing_pixels = 80.0;
    const double center_x = projection.viewport.left + projection.viewport.width * 0.5;
    const double center_y = projection.viewport.top + projection.viewport.height * 0.5;
    const double first_x =
        center_x - std::ceil((center_x - projection.viewport.left) / minor_spacing_pixels) *
                       minor_spacing_pixels;
    const double first_y =
        center_y - std::ceil((center_y - projection.viewport.top) / minor_spacing_pixels) *
                       minor_spacing_pixels;

    sf::VertexArray lines(sf::Lines);
    int index = 0;
    for (double x = first_x; x <= projection.viewport.left + projection.viewport.width;
         x += minor_spacing_pixels, ++index) {
        const sf::Color color = index % 5 == 0 ? style.grid_major : style.grid_minor;
        lines.append(sf::Vertex(sf::Vector2f{as_float(x), rect.top}, color));
        lines.append(sf::Vertex(sf::Vector2f{as_float(x), rect.top + rect.height}, color));
    }

    index = 0;
    for (double y = first_y; y <= projection.viewport.top + projection.viewport.height;
         y += minor_spacing_pixels, ++index) {
        const sf::Color color = index % 5 == 0 ? style.grid_major : style.grid_minor;
        lines.append(sf::Vertex(sf::Vector2f{rect.left, as_float(y)}, color));
        lines.append(sf::Vertex(sf::Vector2f{rect.left + rect.width, as_float(y)}, color));
    }
    target.draw(lines);

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
                   TacticalSelection selection,
                   TacticalMapHighlight highlight,
                   const SfmlTacticalMapStyle& style) {
    for (const auto& contact : snapshot.hostile_contacts) {
        const ScreenPoint screen = world_to_screen(contact.estimated_position_km, projection);
        if (!is_inside(screen, projection)) {
            continue;
        }

        const float uncertainty_radius = std::max(
            2.0F,
            as_float(contact.uncertainty_radius_km / normalized_projection(projection).kilometers_per_pixel));
        sf::CircleShape uncertainty(uncertainty_radius);
        uncertainty.setOrigin(uncertainty_radius, uncertainty_radius);
        uncertainty.setPosition(as_vector(screen));
        uncertainty.setFillColor(sf::Color::Transparent);
        uncertainty.setOutlineColor(style.uncertainty);
        uncertainty.setOutlineThickness(1.0F);
        target.draw(uncertainty);

        sf::CircleShape marker(7.0F, 4);
        marker.setOrigin(7.0F, 7.0F);
        marker.setPosition(as_vector(screen));
        marker.setRotation(45.0F);
        marker.setFillColor(style.contact);
        marker.setOutlineColor(is_selected(selection, contact.id) ? style.selection : style.contact);
        marker.setOutlineThickness(is_selected(selection, contact.id) ? 2.0F : 1.0F);
        target.draw(marker);

        if (highlight.contact == contact.id) {
            draw_circle_marker(target, screen, 12.0F, sf::Color::Transparent, style.hover, 1.0F);
        }
    }
}

void draw_friendlies(sf::RenderTarget& target,
                     const presentation::TacticalSnapshot& snapshot,
                     TacticalMapProjection projection,
                     TacticalSelection selection,
                     TacticalMapHighlight highlight,
                     const SfmlTacticalMapStyle& style) {
    for (const auto& entity : snapshot.friendly_entities) {
        const ScreenPoint screen = world_to_screen(entity.position_km, projection);
        if (!is_inside(screen, projection)) {
            continue;
        }

        draw_circle_marker(target,
                           screen,
                           7.0F,
                           style.friendly,
                           is_selected(selection, entity.id) ? style.selection : style.friendly,
                           is_selected(selection, entity.id) ? 2.0F : 1.0F);

        if (highlight.entity == entity.id) {
            draw_circle_marker(target, screen, 12.0F, sf::Color::Transparent, style.hover, 1.0F);
        }
    }
}

void draw_missiles(sf::RenderTarget& target,
                   const presentation::TacticalSnapshot& snapshot,
                   TacticalMapProjection projection,
                   TacticalMapHighlight highlight,
                   const SfmlTacticalMapStyle& style) {
    for (const auto& missile : snapshot.missile_tracks) {
        const ScreenPoint screen = world_to_screen(missile.position_km, projection);
        if (!is_inside(screen, projection)) {
            continue;
        }

        draw_circle_marker(target, screen, 4.0F, style.missile, style.missile, 1.0F);
        if (highlight.missile == missile.id) {
            draw_circle_marker(target, screen, 9.0F, sf::Color::Transparent, style.hover, 1.0F);
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
    projection = normalized_projection(projection);
    draw_grid(target, projection, style);
    draw_trajectories(target, snapshot, projection, style);
    draw_missiles(target, snapshot, projection, highlight, style);
    draw_contacts(target, snapshot, projection, selection, highlight, style);
    draw_friendlies(target, snapshot, projection, selection, highlight, style);
}

} // namespace scs::rendering
