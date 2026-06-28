#include <iostream>
#include <string>

#include "gameplay/time_scale_policy.h"
#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_renderer.h"
#include "simulation/scenario.h"
#include "simulation/simulation.h"
#include "ui/tactical_command_ui.h"

namespace {

void render_command_log(const scs::ui::TacticalUiState& state) {
    std::cout << "Command log\n";
    if (state.command_log.empty()) {
        std::cout << "  none\n";
        return;
    }

    for (const auto& entry : state.command_log) {
        std::cout << "  " << entry << '\n';
    }
}

} // namespace

int main() {
    auto scenario = scs::simulation::make_playable_engagement_demo_scenario();
    scs::simulation::Simulation simulation(scenario);
    scs::ui::TacticalUiState ui_state;
    ui_state.view = scs::rendering::TacticalMapView{
        scs::domain::Vec2{175.0, 0.0},
        25.0,
        61,
        21,
    };
    ui_state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::FriendlyEntity,
        scs::domain::EntityId{1},
        scs::domain::ContactId{},
    };

    while (true) {
        const auto tactical = scs::presentation::make_tactical_snapshot(
            simulation.snapshot(),
            simulation.events(),
            scs::presentation::TacticalSnapshotOptions{scs::presentation::PredictionConfig{8}});
        const auto time_scale = scs::gameplay::recommend_time_scale(
            tactical.tick,
            tactical.events,
            scs::ui::make_time_scale_input(ui_state));

        std::cout << "\nScenario: " << scenario.name << '\n';
        std::cout << scs::rendering::render_tactical_map(
            tactical,
            ui_state.view,
            ui_state.selection);
        std::cout << "Time scale: " << time_scale.scale << "x ("
                  << scs::gameplay::time_scale_reason_label(time_scale.reason) << ")\n";
        render_command_log(ui_state);
        std::cout << "Command> ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }

        auto result = scs::ui::handle_tactical_input(ui_state, tactical, line);
        if (result.command.has_value()) {
            const bool accepted = simulation.submit(*result.command);
            scs::ui::append_command_log(ui_state,
                                        accepted ? "Simulation accepted command."
                                                 : "Simulation rejected command.");
        }
        if (result.advance_ticks > 0) {
            simulation.advance(result.advance_ticks);
        }
        if (result.quit) {
            break;
        }
    }

    return 0;
}
