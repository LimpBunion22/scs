#include "ui/desktop_time_controls.h"

#include <cmath>
#include <stdexcept>

namespace scs::ui {
namespace {

void validate_config(const DesktopTimeControllerConfig& config) {
    if (!(config.fixed_step_seconds > 0.0)) {
        throw std::invalid_argument("Desktop fixed step must be positive.");
    }
    if (config.max_catch_up_ticks == 0) {
        throw std::invalid_argument("Desktop maximum catch-up ticks must be positive.");
    }
}

} // namespace

DesktopTimeController::DesktopTimeController(DesktopTimeControllerConfig config)
    : config_(config) {
    validate_config(config_);
}

DesktopRunMode DesktopTimeController::mode() const noexcept {
    return mode_;
}

bool DesktopTimeController::is_paused() const noexcept {
    return mode_ == DesktopRunMode::Paused;
}

double DesktopTimeController::accumulated_sim_seconds() const noexcept {
    return accumulated_sim_seconds_;
}

void DesktopTimeController::pause() noexcept {
    mode_ = DesktopRunMode::Paused;
    accumulated_sim_seconds_ = 0.0;
}

void DesktopTimeController::run() noexcept {
    mode_ = DesktopRunMode::Running;
}

void DesktopTimeController::request_step() noexcept {
    step_requested_ = true;
}

domain::Tick DesktopTimeController::request_ticks(
    double elapsed_seconds,
    const gameplay::TimeScaleRecommendation& time_scale) {
    if (!(elapsed_seconds >= 0.0)) {
        throw std::invalid_argument("Desktop elapsed time must be non-negative.");
    }
    if (!(time_scale.scale >= 0.0)) {
        throw std::invalid_argument("Desktop time scale must be non-negative.");
    }

    domain::Tick requested_ticks = 0;
    if (step_requested_) {
        requested_ticks = 1;
        step_requested_ = false;
    }

    if (mode_ != DesktopRunMode::Running || time_scale.scale == 0.0) {
        return requested_ticks;
    }

    accumulated_sim_seconds_ += elapsed_seconds * time_scale.scale;
    const auto available_ticks = static_cast<domain::Tick>(
        std::floor(accumulated_sim_seconds_ / config_.fixed_step_seconds));
    if (available_ticks == 0) {
        return requested_ticks;
    }

    const domain::Tick remaining_capacity =
        requested_ticks >= config_.max_catch_up_ticks
            ? 0
            : config_.max_catch_up_ticks - requested_ticks;
    const domain::Tick continuous_ticks =
        available_ticks > remaining_capacity ? remaining_capacity : available_ticks;

    if (available_ticks > continuous_ticks) {
        accumulated_sim_seconds_ = 0.0;
    } else {
        accumulated_sim_seconds_ -=
            static_cast<double>(continuous_ticks) * config_.fixed_step_seconds;
    }

    return requested_ticks + continuous_ticks;
}

const char* desktop_run_mode_label(DesktopRunMode mode) noexcept {
    switch (mode) {
    case DesktopRunMode::Paused:
        return "Paused";
    case DesktopRunMode::Running:
        return "Running";
    }
    return "Paused";
}

} // namespace scs::ui
