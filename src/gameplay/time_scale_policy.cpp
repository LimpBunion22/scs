#include "gameplay/time_scale_policy.h"

#include <stdexcept>
#include <string>

namespace scs::gameplay {
namespace {

int severity_rank(domain::EventSeverity severity) noexcept {
    switch (severity) {
    case domain::EventSeverity::Info:
        return 0;
    case domain::EventSeverity::Advisory:
        return 1;
    case domain::EventSeverity::Threat:
        return 2;
    case domain::EventSeverity::Critical:
        return 3;
    }
    return 0;
}

void require_positive(double scale, const char* name) {
    if (!(scale > 0.0)) {
        throw std::invalid_argument(std::string("Time-scale ") + name + " must be positive.");
    }
}

void validate_config(const TimeScalePolicyConfig& config) {
    require_positive(config.quiet_scale, "quiet scale");
    require_positive(config.info_scale, "info scale");
    require_positive(config.advisory_scale, "advisory scale");
    require_positive(config.threat_scale, "threat scale");
    require_positive(config.critical_scale, "critical scale");
}

TimeScaleRecommendation recommendation_for(domain::EventSeverity severity,
                                           const TimeScalePolicyConfig& config) noexcept {
    switch (severity) {
    case domain::EventSeverity::Info:
        return TimeScaleRecommendation{config.info_scale, TimeScaleReason::RecentInfo};
    case domain::EventSeverity::Advisory:
        return TimeScaleRecommendation{config.advisory_scale, TimeScaleReason::RecentAdvisory};
    case domain::EventSeverity::Threat:
        return TimeScaleRecommendation{config.threat_scale, TimeScaleReason::RecentThreat};
    case domain::EventSeverity::Critical:
        return TimeScaleRecommendation{config.critical_scale, TimeScaleReason::RecentCritical};
    }
    return TimeScaleRecommendation{config.quiet_scale, TimeScaleReason::Quiet};
}

} // namespace

TimeScaleRecommendation recommend_time_scale(domain::Tick current_tick,
                                             const std::vector<domain::Event>& events,
                                             const PlayerTimeScaleInput& player_input,
                                             const TimeScalePolicyConfig& config) {
    validate_config(config);

    if (player_input.tactical_pause) {
        return TimeScaleRecommendation{0.0, TimeScaleReason::TacticalPause};
    }

    if (player_input.override_scale.has_value()) {
        require_positive(*player_input.override_scale, "player override scale");
        return TimeScaleRecommendation{*player_input.override_scale, TimeScaleReason::PlayerOverride};
    }

    std::optional<domain::EventSeverity> dominant_severity;
    for (const auto& event : events) {
        if (event.tick > current_tick) {
            continue;
        }

        const domain::Tick age = current_tick - event.tick;
        if (age > config.recent_event_window_ticks) {
            continue;
        }

        if (!dominant_severity.has_value() ||
            severity_rank(event.severity) > severity_rank(*dominant_severity)) {
            dominant_severity = event.severity;
        }
    }

    if (!dominant_severity.has_value()) {
        return TimeScaleRecommendation{config.quiet_scale, TimeScaleReason::Quiet};
    }

    return recommendation_for(*dominant_severity, config);
}

const char* time_scale_reason_label(TimeScaleReason reason) noexcept {
    switch (reason) {
    case TimeScaleReason::Quiet:
        return "quiet";
    case TimeScaleReason::RecentInfo:
        return "recent info event";
    case TimeScaleReason::RecentAdvisory:
        return "recent advisory event";
    case TimeScaleReason::RecentThreat:
        return "recent threat event";
    case TimeScaleReason::RecentCritical:
        return "recent critical event";
    case TimeScaleReason::PlayerOverride:
        return "player override";
    case TimeScaleReason::TacticalPause:
        return "tactical pause";
    }
    return "quiet";
}

} // namespace scs::gameplay
