#pragma once

#include "domain/time.h"
#include "gameplay/time_scale_policy.h"

namespace scs::ui {

enum class DesktopRunMode {
    Paused,
    Running,
};

struct DesktopTimeControllerConfig {
    double fixed_step_seconds{1.0};
    domain::Tick max_catch_up_ticks{8};
};

class DesktopTimeController {
public:
    explicit DesktopTimeController(DesktopTimeControllerConfig config = {});

    [[nodiscard]] DesktopRunMode mode() const noexcept;
    [[nodiscard]] bool is_paused() const noexcept;
    [[nodiscard]] double accumulated_sim_seconds() const noexcept;

    void pause() noexcept;
    void run() noexcept;
    void request_step() noexcept;

    [[nodiscard]] domain::Tick request_ticks(
        double elapsed_seconds,
        const gameplay::TimeScaleRecommendation& time_scale);

private:
    DesktopTimeControllerConfig config_;
    DesktopRunMode mode_{DesktopRunMode::Paused};
    bool step_requested_{false};
    double accumulated_sim_seconds_{0.0};
};

[[nodiscard]] const char* desktop_run_mode_label(DesktopRunMode mode) noexcept;

} // namespace scs::ui
