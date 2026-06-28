#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ui/desktop_time_controls.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

scs::gameplay::TimeScaleRecommendation scale(double value) {
    return scs::gameplay::TimeScaleRecommendation{
        value,
        scs::gameplay::TimeScaleReason::Quiet,
    };
}

void paused_state_does_not_advance_from_elapsed_time() {
    scs::ui::DesktopTimeController controller;

    const auto ticks = controller.request_ticks(10.0, scale(1024.0));

    require(controller.mode() == scs::ui::DesktopRunMode::Paused,
            "Controller did not start paused.");
    require(ticks == 0, "Paused controller advanced from elapsed time.");
}

void single_step_works_while_paused() {
    scs::ui::DesktopTimeController controller;
    controller.request_step();

    const auto ticks = controller.request_ticks(0.0, scale(0.0));

    require(ticks == 1, "Step did not request exactly one tick while paused.");
    require(controller.request_ticks(0.0, scale(0.0)) == 0,
            "Step request was not consumed.");
}

void run_mode_accumulates_elapsed_time_into_whole_ticks() {
    scs::ui::DesktopTimeController controller{
        scs::ui::DesktopTimeControllerConfig{1.0, 8}};
    controller.run();

    const auto first = controller.request_ticks(0.25, scale(1.0));
    const auto second = controller.request_ticks(0.75, scale(1.0));

    require(first == 0, "Controller emitted a fractional tick.");
    require(second == 1, "Controller did not accumulate elapsed time into a tick.");
}

void time_scale_multiplier_changes_requested_ticks() {
    scs::ui::DesktopTimeController controller{
        scs::ui::DesktopTimeControllerConfig{1.0, 8}};
    controller.run();

    const auto ticks = controller.request_ticks(0.5, scale(8.0));

    require(ticks == 4, "Time-scale multiplier was not applied.");
}

void fractional_carry_is_retained_below_one_tick() {
    scs::ui::DesktopTimeController controller{
        scs::ui::DesktopTimeControllerConfig{1.0, 8}};
    controller.run();

    require(controller.request_ticks(0.60, scale(1.0)) == 0,
            "Controller emitted a tick too early.");
    require(controller.request_ticks(0.39, scale(1.0)) == 0,
            "Controller ignored fractional carry threshold.");
    require(controller.request_ticks(0.01, scale(1.0)) == 1,
            "Controller did not retain fractional carry.");
}

void maximum_catch_up_clamp_limits_large_frames() {
    scs::ui::DesktopTimeController controller{
        scs::ui::DesktopTimeControllerConfig{1.0, 3}};
    controller.run();

    const auto ticks = controller.request_ticks(10.0, scale(10.0));

    require(ticks == 3, "Catch-up clamp did not limit requested ticks.");
    require(controller.accumulated_sim_seconds() == 0.0,
            "Catch-up clamp retained excess backlog.");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests{
        {"paused_state_does_not_advance_from_elapsed_time",
         paused_state_does_not_advance_from_elapsed_time},
        {"single_step_works_while_paused", single_step_works_while_paused},
        {"run_mode_accumulates_elapsed_time_into_whole_ticks",
         run_mode_accumulates_elapsed_time_into_whole_ticks},
        {"time_scale_multiplier_changes_requested_ticks",
         time_scale_multiplier_changes_requested_ticks},
        {"fractional_carry_is_retained_below_one_tick",
         fractional_carry_is_retained_below_one_tick},
        {"maximum_catch_up_clamp_limits_large_frames",
         maximum_catch_up_clamp_limits_large_frames},
    };

    for (const auto& test : tests) {
        try {
            test.second();
            std::cout << "[PASS] " << test.first << '\n';
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << test.first << ": " << error.what() << '\n';
            return 1;
        }
    }

    return 0;
}
