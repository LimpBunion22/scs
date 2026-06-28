#pragma once

#include <vector>

#include "domain/time.h"
#include "domain/vector2.h"

namespace scs::presentation {

struct PredictionConfig {
    domain::Tick horizon_ticks{0};
};

struct TrajectoryPoint {
    domain::Tick tick{0};
    double time_seconds{0.0};
    domain::Vec2 position_km;
};

[[nodiscard]] std::vector<TrajectoryPoint> predict_inertial_trajectory(
    domain::Tick start_tick,
    double start_time_seconds,
    domain::Vec2 start_position_km,
    domain::Vec2 velocity_km_per_second,
    double seconds_per_tick,
    PredictionConfig config);

} // namespace scs::presentation
