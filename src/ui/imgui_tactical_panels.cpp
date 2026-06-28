#include "ui/imgui_tactical_panels.h"

#include <string>

#include <imgui.h>

#include "ui/desktop_order_model.h"
#include "ui/desktop_panel_model.h"

namespace scs::ui {
namespace {

void draw_metric_table(const char* table_id,
                       const std::vector<DesktopPanelMetric>& metrics) {
    if (metrics.empty()) {
        ImGui::TextUnformatted("None");
        return;
    }

    if (!ImGui::BeginTable(table_id, 2, ImGuiTableFlags_SizingFixedFit)) {
        return;
    }

    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 110.0F);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

    for (const auto& metric : metrics) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(metric.label.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::TextWrapped("%s", metric.value.c_str());
    }

    ImGui::EndTable();
}

void draw_selection(const DesktopPanelSelectionModel& selection) {
    ImGui::SeparatorText("Selection");
    ImGui::TextWrapped("%s", selection.heading.c_str());
    draw_metric_table("selection_metrics", selection.metrics);
}

void draw_hover(const DesktopPanelModel& model) {
    ImGui::SeparatorText("Hover");
    ImGui::TextWrapped("%s", model.hover_summary.c_str());
}

void draw_staged_engagement(const DesktopPanelModel& model) {
    ImGui::SeparatorText("Engagement");

    std::vector<DesktopPanelMetric> staged_metrics;
    staged_metrics.reserve(model.staged_statuses.size());
    for (const auto& status : model.staged_statuses) {
        staged_metrics.push_back({status.label, status.value + " (" + status.status + ")"});
    }
    draw_metric_table("staged_metrics", staged_metrics);
}

void draw_maneuver_order(const DesktopVelocityOrderModel& model,
                         DesktopInteractionState& desktop_state) {
    ImGui::SeparatorText("Maneuver");
    ImGui::TextWrapped("Target %s",
                       model.target_label.empty() ? "none" : model.target_label.c_str());
    ImGui::TextWrapped("%s", model.feedback.c_str());
    ImGui::InputDouble("VX (km/s)", &desktop_state.staged_velocity_x_km_per_second, 1.0, 10.0);
    ImGui::InputDouble("VY (km/s)", &desktop_state.staged_velocity_y_km_per_second, 1.0, 10.0);
}

void draw_missiles(const DesktopPanelModel& model) {
    ImGui::SeparatorText("Missiles");
    if (model.missiles.empty()) {
        ImGui::TextUnformatted("None");
        return;
    }

    for (std::size_t index = 0; index < model.missiles.size(); ++index) {
        const auto& missile = model.missiles[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TextUnformatted(missile.heading.c_str());
        draw_metric_table("missile_metrics", missile.metrics);
        ImGui::PopID();
    }
}

void draw_events(const DesktopPanelModel& model) {
    ImGui::SeparatorText("Events");
    if (model.event_lines.empty()) {
        ImGui::TextUnformatted("None");
        return;
    }

    for (const auto& line : model.event_lines) {
        ImGui::TextWrapped("%s", line.c_str());
    }
}

void draw_command_log(const DesktopPanelModel& model) {
    ImGui::SeparatorText("Command Log");
    if (model.command_log_lines.empty()) {
        ImGui::TextUnformatted("None");
        return;
    }

    for (const auto& entry : model.command_log_lines) {
        ImGui::TextWrapped("%s", entry.c_str());
    }
}

} // namespace

DesktopCommandResult draw_imgui_tactical_panels(
    TacticalUiState& ui_state,
    DesktopInteractionState& desktop_state,
    const presentation::TacticalSnapshot& snapshot,
    const gameplay::TimeScaleRecommendation& time_scale,
    DesktopTimeController& time_controller,
    std::string_view scenario_name,
    ImguiPanelLayout layout) {
    DesktopCommandResult result;

    ImGui::SetNextWindowPos(ImVec2{layout.left, layout.top}, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2{layout.width, layout.height}, ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Tactical Command", nullptr, flags);

    const DesktopPanelModel model =
        make_desktop_panel_model(ui_state, desktop_state, snapshot, time_scale);
    const std::string scenario{scenario_name};
    ImGui::TextWrapped("%s", scenario.c_str());
    ImGui::Text("Tick %llu  Time %.1fs",
                static_cast<unsigned long long>(snapshot.tick),
                snapshot.time_seconds);
    ImGui::TextWrapped("Scale %s  %s",
                       model.time_scale_label.c_str(),
                       model.time_scale_reason.c_str());

    ImGui::SeparatorText("Time");
    ImGui::Text("Mode %s", desktop_run_mode_label(time_controller.mode()));
    if (ImGui::Button("Pause")) {
        time_controller.pause();
        ui_state.tactical_pause = true;
        result.feedback = "Tactical pause enabled.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Run")) {
        time_controller.run();
        ui_state.tactical_pause = false;
        result.feedback = "Continuous run enabled.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        time_controller.request_step();
        result.feedback = "Simulation step requested.";
    }

    if (ImGui::Button("Auto Scale")) {
        ui_state.manual_time_scale.reset();
        result.feedback = "Automatic time scale restored.";
    }
    ImGui::SameLine();
    if (ImGui::Button("1x")) {
        ui_state.manual_time_scale = 1.0;
        result.feedback = "Manual time scale override set.";
    }
    ImGui::SameLine();
    if (ImGui::Button("8x")) {
        ui_state.manual_time_scale = 8.0;
        result.feedback = "Manual time scale override set.";
    }
    ImGui::SameLine();
    if (ImGui::Button("64x")) {
        ui_state.manual_time_scale = 64.0;
        result.feedback = "Manual time scale override set.";
    }

    draw_selection(model.selection);
    draw_hover(model);

    const DesktopVelocityOrderModel velocity_order =
        make_desktop_velocity_order_model(desktop_state, snapshot);
    draw_maneuver_order(velocity_order, desktop_state);
    if (ImGui::Button("Set Velocity")) {
        result = emit_desktop_velocity_command(desktop_state, snapshot);
    }

    draw_staged_engagement(model);
    if (ImGui::Button("Engage Contact")) {
        result = emit_staged_desktop_engage_contact(desktop_state, snapshot);
    }

    draw_missiles(model);
    draw_events(model);
    draw_command_log(model);

    if (ImGui::Button("Quit")) {
        result.quit = true;
        result.feedback = "Quit requested.";
        result.advance_ticks = 0;
    }

    ImGui::End();
    return result;
}

} // namespace scs::ui
