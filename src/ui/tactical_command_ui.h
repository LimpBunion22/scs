#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "domain/command.h"
#include "gameplay/time_scale_policy.h"
#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_renderer.h"

namespace scs::ui {

struct TacticalUiState {
    rendering::TacticalMapView view;
    rendering::TacticalSelection selection;
    bool tactical_pause{false};
    std::optional<double> manual_time_scale;
    std::vector<std::string> command_log;
};

struct TacticalInputResult {
    bool quit{false};
    domain::Tick advance_ticks{0};
    std::optional<domain::Command> command;
    std::string feedback;
};

[[nodiscard]] std::string tactical_command_help();
[[nodiscard]] gameplay::PlayerTimeScaleInput make_time_scale_input(const TacticalUiState& state);
[[nodiscard]] TacticalInputResult handle_tactical_input(
    TacticalUiState& state,
    const presentation::TacticalSnapshot& snapshot,
    std::string_view line);

void append_command_log(TacticalUiState& state, std::string entry);

} // namespace scs::ui
