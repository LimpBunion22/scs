#include "presentation/prediction.h"

#include <stdexcept>

namespace scs::presentation {

std::vector<TrajectoryPoint> predict_inertial_trajectory(domain::Tick start_tick,
                                                         double start_time_seconds,
                                                         domain::Vec2 start_position_km,
                                                         domain::Vec2 velocity_km_per_second,
                                                         double seconds_per_tick,
                                                         PredictionConfig config) {
    if (seconds_per_tick <= 0.0) {
        throw std::invalid_argument("Prediction seconds per tick must be positive.");
    }

    std::vector<TrajectoryPoint> result;
    result.reserve(static_cast<std::size_t>(config.horizon_ticks + 1));

    domain::Tick offset = 0;
    while (true) {
        const double elapsed_seconds = static_cast<double>(offset) * seconds_per_tick;
        result.push_back(TrajectoryPoint{
            start_tick + offset,
            start_time_seconds + elapsed_seconds,
            start_position_km + velocity_km_per_second * elapsed_seconds,
        });

        if (offset == config.horizon_ticks) {
            break;
        }
        ++offset;
    }

    return result;
}

} // namespace scs::presentation
