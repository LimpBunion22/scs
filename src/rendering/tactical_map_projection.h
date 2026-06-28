#pragma once

#include "domain/vector2.h"

namespace scs::rendering {

struct ScreenPoint {
    double x{0.0};
    double y{0.0};
};

struct ScreenRect {
    double left{0.0};
    double top{0.0};
    double width{1.0};
    double height{1.0};
};

struct TacticalMapProjection {
    domain::Vec2 center_km;
    double kilometers_per_pixel{1.0};
    ScreenRect viewport;
};

[[nodiscard]] TacticalMapProjection normalized_projection(TacticalMapProjection projection);
[[nodiscard]] ScreenPoint world_to_screen(domain::Vec2 world_km,
                                          TacticalMapProjection projection);
[[nodiscard]] domain::Vec2 screen_to_world(ScreenPoint screen,
                                           TacticalMapProjection projection);

void pan_projection_by_pixels(TacticalMapProjection& projection, ScreenPoint delta_pixels);
void zoom_projection_around_screen_point(TacticalMapProjection& projection,
                                         ScreenPoint anchor,
                                         double scale_factor);

} // namespace scs::rendering
