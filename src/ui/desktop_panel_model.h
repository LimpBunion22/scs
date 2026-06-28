#pragma once

#include <string>
#include <vector>

#include "gameplay/time_scale_policy.h"
#include "presentation/tactical_snapshot.h"
#include "ui/desktop_interaction.h"
#include "ui/tactical_command_ui.h"

namespace scs::ui {

struct DesktopPanelMetric {
    std::string label;
    std::string value;
};

struct DesktopPanelSelectionModel {
    rendering::TacticalSelectionKind kind{rendering::TacticalSelectionKind::None};
    std::string heading;
    std::vector<DesktopPanelMetric> metrics;
};

struct DesktopPanelStagedStatus {
    std::string label;
    std::string value;
    std::string status;
};

struct DesktopPanelMissileModel {
    std::string heading;
    std::vector<DesktopPanelMetric> metrics;
};

struct DesktopPanelModel {
    std::string time_scale_label;
    std::string time_scale_reason;
    std::string hover_summary;
    DesktopPanelSelectionModel selection;
    std::vector<DesktopPanelStagedStatus> staged_statuses;
    std::vector<DesktopPanelMissileModel> missiles;
    std::vector<std::string> event_lines;
    std::vector<std::string> command_log_lines;
};

[[nodiscard]] DesktopPanelModel make_desktop_panel_model(
    const TacticalUiState& ui_state,
    const DesktopInteractionState& desktop_state,
    const presentation::TacticalSnapshot& snapshot,
    const gameplay::TimeScaleRecommendation& time_scale);

} // namespace scs::ui
