#pragma once

#include <optional>
#include <vector>

#include "domain/event.h"
#include "domain/time.h"

namespace scs::gameplay {

enum class TimeScaleReason {
    Quiet,
    RecentInfo,
    RecentAdvisory,
    RecentThreat,
    RecentCritical,
    PlayerOverride,
    TacticalPause
};

struct TimeScalePolicyConfig {
    domain::Tick recent_event_window_ticks{30};
    double quiet_scale{1024.0};
    double info_scale{256.0};
    double advisory_scale{64.0};
    double threat_scale{8.0};
    double critical_scale{1.0};
};

struct PlayerTimeScaleInput {
    bool tactical_pause{false};
    std::optional<double> override_scale;
};

struct TimeScaleRecommendation {
    double scale{1.0};
    TimeScaleReason reason{TimeScaleReason::Quiet};
};

[[nodiscard]] TimeScaleRecommendation recommend_time_scale(
    domain::Tick current_tick,
    const std::vector<domain::Event>& events,
    const PlayerTimeScaleInput& player_input = PlayerTimeScaleInput{},
    const TimeScalePolicyConfig& config = TimeScalePolicyConfig{});

[[nodiscard]] const char* time_scale_reason_label(TimeScaleReason reason) noexcept;

} // namespace scs::gameplay
