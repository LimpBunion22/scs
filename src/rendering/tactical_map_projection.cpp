#include "rendering/tactical_map_projection.h"

#include <algorithm>

namespace scs::rendering {
namespace {

constexpr double kMinimumViewportExtent = 1.0;
constexpr double kMinimumKilometersPerPixel = 0.001;
constexpr double kMaximumKilometersPerPixel = 10'000'000.0;

} // namespace

TacticalMapProjection normalized_projection(TacticalMapProjection projection) {
    projection.viewport.width = std::max(kMinimumViewportExtent, projection.viewport.width);
    projection.viewport.height = std::max(kMinimumViewportExtent, projection.viewport.height);
    projection.kilometers_per_pixel = std::min(
        kMaximumKilometersPerPixel,
        std::max(kMinimumKilometersPerPixel, projection.kilometers_per_pixel));
    return projection;
}

ScreenPoint world_to_screen(domain::Vec2 world_km, TacticalMapProjection projection) {
    projection = normalized_projection(projection);
    const double center_x = projection.viewport.left + projection.viewport.width * 0.5;
    const double center_y = projection.viewport.top + projection.viewport.height * 0.5;
    return ScreenPoint{
        center_x + (world_km.x - projection.center_km.x) / projection.kilometers_per_pixel,
        center_y - (world_km.y - projection.center_km.y) / projection.kilometers_per_pixel,
    };
}

domain::Vec2 screen_to_world(ScreenPoint screen, TacticalMapProjection projection) {
    projection = normalized_projection(projection);
    const double center_x = projection.viewport.left + projection.viewport.width * 0.5;
    const double center_y = projection.viewport.top + projection.viewport.height * 0.5;
    return domain::Vec2{
        projection.center_km.x + (screen.x - center_x) * projection.kilometers_per_pixel,
        projection.center_km.y - (screen.y - center_y) * projection.kilometers_per_pixel,
    };
}

void pan_projection_by_pixels(TacticalMapProjection& projection, ScreenPoint delta_pixels) {
    projection = normalized_projection(projection);
    projection.center_km.x -= delta_pixels.x * projection.kilometers_per_pixel;
    projection.center_km.y += delta_pixels.y * projection.kilometers_per_pixel;
}

void zoom_projection_around_screen_point(TacticalMapProjection& projection,
                                         ScreenPoint anchor,
                                         double scale_factor) {
    projection = normalized_projection(projection);
    if (scale_factor <= 0.0) {
        return;
    }

    const domain::Vec2 anchored_world = screen_to_world(anchor, projection);
    projection.kilometers_per_pixel *= scale_factor;
    projection = normalized_projection(projection);

    const double center_x = projection.viewport.left + projection.viewport.width * 0.5;
    const double center_y = projection.viewport.top + projection.viewport.height * 0.5;
    projection.center_km = domain::Vec2{
        anchored_world.x - (anchor.x - center_x) * projection.kilometers_per_pixel,
        anchored_world.y + (anchor.y - center_y) * projection.kilometers_per_pixel,
    };
}

} // namespace scs::rendering
