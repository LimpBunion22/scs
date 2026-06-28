#pragma once

#include <string>
#include <string_view>

#include "gameplay/time_scale_policy.h"
#include "presentation/tactical_snapshot.h"
#include "ui/desktop_interaction.h"
#include "ui/desktop_time_controls.h"
#include "ui/tactical_command_ui.h"

namespace scs::ui {

struct ImguiPanelLayout {
    float left{0.0F};
    float top{0.0F};
    float width{360.0F};
    float height{720.0F};
};

[[nodiscard]] DesktopCommandResult draw_imgui_tactical_panels(
    TacticalUiState& ui_state,
    DesktopInteractionState& desktop_state,
    const presentation::TacticalSnapshot& snapshot,
    const gameplay::TimeScaleRecommendation& time_scale,
    DesktopTimeController& time_controller,
    std::string_view scenario_name,
    ImguiPanelLayout layout);

} // namespace scs::ui
