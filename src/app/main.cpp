#include <iostream>

#include "domain/command.h"
#include "simulation/replay.h"
#include "simulation/scenario.h"

int main() {
    auto scenario = scs::simulation::make_default_vertical_slice_scenario();
    scs::simulation::ReplayInput replay{
        scenario,
        {
            scs::domain::set_velocity_at(
                5,
                scs::domain::EntityId{1},
                scs::domain::Vec2{20.0, 0.5}),
        },
        10,
    };

    const auto result = scs::simulation::run_replay(replay);

    std::cout << "Scenario: " << scenario.name << '\n';
    std::cout << "Tick: " << result.final_snapshot.tick
              << " Time: " << result.final_snapshot.time_seconds << "s\n";
    for (const auto& entity : result.final_snapshot.entities) {
        std::cout << entity.id.value << " " << entity.name << " position_km=("
                  << entity.position_km.x << ", " << entity.position_km.y << ")\n";
    }

    return 0;
}
